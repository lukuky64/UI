#include "Controller.h"

Controller::Controller() : running(false), calibrationRunning(false), sensorInitialised(false), sdInitialised(false), dataInitialised(false), gainScheduleInitialised(false), Kp(1), Ki(0), Kd(0), alpha(0.5), logFreq(20)
{
    logTime = 1000000 / logFreq; // convert to microseconds

    control_pid.SetMode(AUTOMATIC);
    control_pid.SetOutputLimits(-100, 100); // 0-100% speed, sign indicates direction
}

bool Controller::initDevices(float alpha_)
{
    sdInitialised = sd.init(SD_CS);
    initSensor(alpha_);

    return sdInitialised && sensorInitialised;
}

Controller::~Controller()
{
    // Ensure the controller is stopped
    if (running)
    {
        stop();
    }
}

bool Controller::initData(sim_data &data_)
{
    // initialise gain schedules
    initGainSchedule();

    data = &data_;
    dataInitialised = true;

    // DBG("data: " + String(dataInitialised) + " sensor: " + String(sensorInitialised) + " sd: " + String(sdInitialised));

    return dataInitialised && sensorInitialised && gainScheduleInitialised;
}

bool Controller::initGainSchedule()
{
    // there should be a file on the SD card that contains the gain schedules for the controller
    // the file should be in the format:
    // Kp, Ki, Kd, pressure

    // read the file and populate the gain schedule array

    if (sdInitialised)
    {
        gainScheduleInitialised = sd.loadGainsFromFile("gains.csv", gainSchedule);
        if (!gainScheduleInitialised)
        {
            DBG("Failed to load gain schedule from SD card");
        }
    }
    else
    {
        DBG("SD card not initialised");
        gainScheduleInitialised = false;
    }

    return gainScheduleInitialised;
}

bool Controller::initSensor(float alpha_)
{
    if (!sensorInitialised)
    {
        filteredReading = 101325; // initial guess of sea level pressure. We want to reset it here upon reuse
        setAlpha(alpha_);
        sensorInitialised = pressureSensor.begin(I2C_SDA, I2C_SCL, 0x76); // initialise BMP280 sensor
    }

    if (sensorInitialised)
    {
        DBG("Sensor initialised");
    }
    else
    {
        DBG("Sensor failed to initialise");
    }

    return sensorInitialised;
}

bool Controller::run()
{
    if (sensorInitialised && dataInitialised && gainScheduleInitialised)
    {
        running = true;
        startMillis = millis();
        return true;
    }
    else
    {
        return false;
    }
}

bool Controller::initCalibrateSystem(float setPointPressure)
{
    bool initialised = false;
    calibrationSetPointPressure = setPointPressure; // set the setpoint for calibration
    // DBG(calibrationSetPointPressure);

    calibrationState = ground; // set initial state

    bool fileCreated = sd.createFile("state, time, pressure", "cal");

    DBG("sensor: " + String(sensorInitialised) + " sd: " + String(sdInitialised) + " file: " + String(fileCreated));

    initialised = sensorInitialised && sdInitialised && fileCreated;

    return initialised;
}

bool Controller::startCalibrateSystem()
{
    if (sensorInitialised && sdInitialised)
    {
        if ((!calibrationRunning))
        {
            calibrationRunning = true;
            startMillis = millis();
        }

        if (!LogDesiredData(String(calibrationState), true))
        {
            DBG("Failed to log data to SD");
            calibrationRunning = false;
        }
    }
    return calibrationRunning;
}

bool Controller::LogDesiredData(String firstData, bool forceLog)
{
    if (forceLog)
    {
        String dataEntry = firstData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
        lastLogTime = micros();
        return sd.writeToBuffer(dataEntry);
    }
    else
    {
        timePassed = micros() - lastLogTime;
        if (timePassed >= logTime)
        {
            String dataEntry = firstData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
            lastLogTime = micros();
            return sd.writeToBuffer(dataEntry);
        }
    }
    return true; // will only return false if failed to write to buffer
}

bool Controller::calibrateIterate()
{
    bool calibrating = true;

    if (calibrationRunning && updateReading())
    {
        // updateGains();

        currentSeconds = (float(millis()) - float(startMillis)) / 1000.0f; // time since start in seconds

        if (calibrationState == ground)
        {
            if (currentSeconds >= 5)
            {
                pump.sendCommand(100); // pump max speed
                calibrationState = pumping;
            }
        }
        else if (calibrationState == pumping)
        {
            calibrationProgress = 0.5 * (1 - ((Input - calibrationSetPointPressure) / (pressureSensor.getBasePressure() - calibrationSetPointPressure)));

            if (Input <= calibrationSetPointPressure)
            {
                pump.sendCommand(0);
                calibrationState = leaking;
            }
        }
        else if (calibrationState == leaking)
        {
            calibrationProgress = 0.5 * (1 + ((Input - calibrationSetPointPressure) / (pressureSensor.getBasePressure() - calibrationSetPointPressure)));

            if (Input >= (pressureSensor.getBasePressure() - 50)) // take of a little bit of pressure to account for noise and drift
            {
                stop();
                calibrationProgress = 1;
                calibrationRunning = false;
                calibrating = false;
            }
        }
        // DBG(pressureSensor.getBasePressure());
        if (!LogDesiredData(String(calibrationState), false))
        {
            DBG("Failed to log data to SD");
            calibrationRunning = false;
            calibrating = false;
        }
    }
    return calibrating;
}

void Controller::updateGains()
{
    // gain schedule array should be sorted from lowest to highest operating pressure
    // for efficiency but also to ensure that the correct gains are selected
    if (!gainScheduleInitialised)
    {
        DBG("Gain schedule not initialised");
        return;
    }

    for (int i = 0; i < gainSchedule.height; i++)
    {
        if (Input >= gainSchedule.data[i][3])
        {
            Kp = gainSchedule.data[i][0];
            Ki = gainSchedule.data[i][1];
            Kd = gainSchedule.data[i][2];

            control_pid.SetTunings(Kp, Ki, Kd);
            break;
        }
    }
}

void Controller::stop()
{
    calibrationRunning = false;
    running = false;
    pump.sendCommand(0);
}

float Controller::getCalibrationProgress()
{
    return calibrationProgress;
}

bool Controller::updateReading()
{
    float rawReading = pressureSensor.getPressure(true);

    if (rawReading == -1)
    {
        DBG("Failed to get pressure reading");
        return false;
    }

    filteredReading = ((1 - alpha) * rawReading) + (alpha * filteredReading);
    Input = filteredReading;

    return true;
}

void Controller::setAlpha(float alpha_)
{
    // can't be be 1 because reading will never change
    alpha = constrain(alpha_, 0, 0.99);
}

float Controller::getAlpha()
{
    return alpha;
}

bool Controller::iterate()
{
    int i = 1; // first value seems to always be 0 for pressure which causes issues

    if (running)
    {
        currentSeconds = (float(millis()) - float(startMillis)) / 1000.0f; // time since start in seconds

        running = updateReading();

        // find closest time in data
        for (i; i < data->num_points; i++)
        {
            if (data->time[i] >= currentSeconds)
            {
                break;
            }
            if (i == data->num_points - 1)
            {
                running = false;
            }
        }

        Setpoint = float(data->pressure[i]);

        updateGains();
        control_pid.Compute();

        pump.sendCommand(Output);

        i++;
        return true;
    }
    else
    {
        DBG("Controller not running");
        return false;
    }
}

float Controller::getLatestTime()
{
    return currentSeconds;
}

float Controller::getLatestPressure()
{
    return Input;
}

float Controller::getLatestSetpoint()
{
    return float(Setpoint);
}
