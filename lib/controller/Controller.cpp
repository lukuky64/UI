#include "Controller.h"

Controller::Controller() : running(false), calibrationRunning(false), sensorInitialised(false), sdInitialised(false), dataInitialised(false), Kp(0.1), Ki(0), Kd(0), logTime(100000)
{
    control_pid.SetMode(AUTOMATIC);
    control_pid.SetOutputLimits(-100, 100); // 0-100% speed, sign indicates direction
}

Controller::~Controller()
{
    // Ensure the controller is stopped
    if (running)
    {
        stop();
    }
}

bool Controller::init(sim_data &data_)
{
    dataInitialised = false;

    initSensor();

    if (sensorInitialised)
    {
        if (sensorInitialised)
        {
            data = &data_;
            dataInitialised = true;
        }
    }

    return dataInitialised;
}

bool Controller::initSensor()
{
    if (!sensorInitialised)
    {
        filteredReading = 101325; // initial guess of sea level pressure. We want to reset it here upon reuse
        alpha = 0.006;
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
    if (sensorInitialised && dataInitialised)
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

bool Controller::initSD(u_int8_t CS, String text)
{
    sdInitialised = sd.init(CS, text, "cal");

    if (sdInitialised)
    {
        DBG("sd successful");
    }
    else
    {
        DBG("sd failed");
    }

    return sdInitialised;
}

bool Controller::initCalibrateSystem(float setPointPressure)
{
    bool initialised = false;
    calibrationSetPointPressure = setPointPressure; // set the setpoint for calibration
    // DBG(calibrationSetPointPressure);

    calibrationState = ground; // set initial state

    initSensor();
    initSD(SD_CS, "state, time, pressure");

    initialised = sensorInitialised && sdInitialised;

    return initialised;
}

bool Controller::startCalibrateSystem()
{
    if ((!calibrationRunning) && sensorInitialised && sdInitialised)
    {
        calibrationRunning = true;
        startMillis = millis();
    }

    if (!LogDesiredData(String(calibrationState), true))
    {
        DBG("Failed to log data to SD");
        calibrationRunning = false;
    }

    return calibrationRunning;
}

bool Controller::LogDesiredData(String firstData, bool forceLog)
{
    timePassed = micros() - lastLogTime;

    if (forceLog)
    {
        String dataEntry = firstData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
        lastLogTime = micros();
        return sd.writeToBuffer(dataEntry);
    }
    else
    {
        if (timePassed >= logTime)
        {
            String dataEntry = firstData + String(",") + String(currentSeconds) + String(",") + String(Input) + String("\n");
            lastLogTime = micros();
            return sd.writeToBuffer(dataEntry);
        }
    }
    return false;
}

bool Controller::calibrateIterate()
{
    bool calibrating = true;

    if (calibrationRunning && updateReading())
    {
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

    filteredReading = alpha * rawReading + (1 - alpha) * filteredReading;
    Input = filteredReading;

    return true;
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

        i++;

        control_pid.Compute();

        pump.sendCommand(Output);

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
