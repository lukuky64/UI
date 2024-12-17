#include "Controller.h"
#include <algorithm>

Controller::Controller() : running(false), calibrationRunning(false), sensorInitialised(false), sdInitialised(false), dataInitialised(false), gainScheduleInitialised(false), alpha(0.5), logFreq(20)
{
    logTime = 1000000 / logFreq; // convert to microseconds
}

bool Controller::initDevices(float alpha_)
{
    sdInitialised = sd.init(SD_CS);
    sensorInitialised = initSensor(alpha_);

    return sdInitialised && sensorInitialised;
}

void Controller::calibrateBasePressure()
{
    pressureSensor.calibrateBasePressure();
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
        gainScheduleInitialised = sd.loadGainsFromFile("/CONTROL/gains.csv", gainSchedule);
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
    filteredReading = 101325; // initial guess of sea level pressure. We want to reset it here upon reuse

    if (sensorInitialised)
    {
        DBG("Testing sensor connection");
        sensorInitialised = pressureSensor.testConnection();
        return sensorInitialised;
    }

    // removed the check for sensorInitialised because we want to reset the sensor

    setAlpha(alpha_);
    sensorInitialised = pressureSensor.begin(I2C_SDA, I2C_SCL, 0x76); // initialise BMP280 sensor

    if (sensorInitialised)
    {
        DBG("Sensor initialised");
        return true;
    }
    else
    {
        DBG("Sensor failed to initialise");
        return false;
    }
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

    String alpha_str = String((uint8_t)(alpha * 100)); // Convert float to String
    alpha_str.replace('.', '_');                       // Replace '.' with '_'

    bool fileCreated = sd.createFile("state, time, pressure", String("/CALIB/a_" + alpha_str));

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
            currentSeconds = 0;
        }

        if (!updateReading())
        {
            DBG("Failed to get pressure reading");
            calibrationRunning = false;
        }

        if (!LogDesiredData(String(calibrationState), true))
        {
            DBG("Failed to log data to SD");

            calibrationRunning = false;
        }
    }
    return calibrationRunning;
}

bool Controller::LogDesiredData(String stateData, bool forceLog)
{
    if (forceLog)
    {
        String dataEntry = stateData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
        lastLogTime = micros();
        return sd.writeToBuffer(dataEntry);
    }
    else
    {
        timePassed = micros() - lastLogTime;
        if (timePassed >= logTime)
        {
            String dataEntry = stateData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
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
        calibrating = (Input >= (safePressureLow) && Input <= (safePressureHigh)); // check if reading is within bounds (-100m to 10,000m)

        if (!calibrating)
        {
            pump.sendCommand(0.0);
            calibrationRunning = false;
            calibrationProgress = 0;

            DBG("Failed to get pressure reading or out of bounds");
            return false;
        }

        currentSeconds = (float(millis()) - float(startMillis)) / 1000.0f; // time since start in seconds

        if (calibrationState == ground)
        {
            if (currentSeconds >= 5)
            {
                pump.sendCommand(100.0); // pump max speed
                calibrationState = pumping;
            }
        }
        else if (calibrationState == pumping)
        {
            calibrationProgress = 0.5 * (1 - ((Input - calibrationSetPointPressure) / (pressureSensor.getBasePressure() - calibrationSetPointPressure)));

            if (Input <= calibrationSetPointPressure)
            {
                pump.sendCommand(0.0);
                calibrationState = leaking;
            }
        }
        else if (calibrationState == leaking)
        {
            calibrationProgress = 0.5 * (1 + max(0.0, ((Input - calibrationSetPointPressure) / (pressureSensor.getBasePressure() - calibrationSetPointPressure))));

            if (Input >= (pressureSensor.getBasePressure() - 100)) // take of a little bit of pressure to account for noise and drift
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

bool Controller::updateGains()
{
    // gain schedule array should be sorted from lowest to highest operating pressure
    // for efficiency but also to ensure that the correct gains are selected
    if (!gainScheduleInitialised)
    {
        DBG("Gain schedule not initialised");
        return false;
    }

    // initialise gains incase we don't find a match

    Kp = abs(gainSchedule.data[-1][0]);
    Ki = abs(gainSchedule.data[-1][1]);
    Kd = abs(gainSchedule.data[-1][2]);

    for (int i = 0; i < gainSchedule.height; i++)
    {
        if (Input <= gainSchedule.data[i][3])
        {
            // must be positive values
            Kp = abs(gainSchedule.data[i][0]);
            Ki = abs(gainSchedule.data[i][1]);
            Kd = abs(gainSchedule.data[i][2]);

            DBG("match found at: " + String(i));

            break;
        }
    }

    // DBG("Kp: " + String(Kp, 6) + " Ki: " + String(Ki, 6) + " Kd: " + String(Kd, 6));
    control_pid.SetTunings(Kp, Ki, Kd);
    return true;
}

void Controller::stop()
{
    calibrationRunning = false;
    running = false;
    pump.sendCommand(0.0);
}

float Controller::getCalibrationProgress()
{
    return calibrationProgress;
}

void Controller::setCalibrationProgress(float calibrationProgress_)
{
    calibrationProgress = calibrationProgress_;
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
    alpha = constrain(alpha_, 0.2, 0.99);
}

void Controller::initPID()
{
    control_pid.SetMode(AUTOMATIC);
    control_pid.SetOutputLimits(-100, 0); // 0-100% speed, sign indicates direction. pump can only suck so output is between 0 and 100
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

        running = (Input >= (safePressureLow) && Input <= (safePressureHigh)); // check if reading is within bounds (-100m to 10,000m)

        if (!running)
        {
            pump.sendCommand(0.0);
            DBG("Failed to get pressure reading or out of bounds");
            return false;
        }

        // find closest time in data
        for (i; i < data->num_points; i++)
        {
            if (data->time[i] >= currentSeconds)
            {
                i++;
                break;
            }
        }

        if (data->time[data->num_points] <= currentSeconds)
        {
            pump.sendCommand(0.0);
            running = false;
            return running;
        }

        Setpoint = float(data->pressure[i]);

        running = updateGains();
        control_pid.Compute();

        // DBG("Setpoint: " + String(Setpoint) + " Input: " + String(Input) + " Output: " + String(Output));

        pump.sendCommand(Output);

        return true;
    }
    else
    {
        pump.sendCommand(0.0);
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
