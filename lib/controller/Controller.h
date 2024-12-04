#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Arduino.h"
#include "pressureSensor.h"
#include "Pump.h"
#include "DataType.h"
#include "PID_v1.hpp"
#include "Debug.hpp"
#include "SD.hpp"
#include "gainScheduleData.h"

class Controller
{
public:
    Controller();
    ~Controller();
    bool init(sim_data &data_);
    bool run();
    void stop();
    bool iterate();
    float getLatestTime();
    float getLatestPressure();
    float getLatestSetpoint();
    bool calibrateSystem(float setPoint);
    bool initCalibrateSystem(float setPoint);
    bool LogDesiredData(String firstData, bool forceLog);
    bool startCalibrateSystem();
    bool calibrateIterate();
    bool updateReading();
    bool initSensor(float alpha_);
    float getCalibrationProgress();
    void updateGains();
    bool initGainSchedule();

private:
    Pump pump;
    PressureSensor pressureSensor;
    sim_data *data = nullptr; // Pointer to sim_data

    float filteredReading; // initial guess of sea level pressure
    float alpha;           // high alpha means more weight to new data

    // PID control
    double Setpoint, Input, Output;
    double Kp;
    double Ki;
    double Kd;

    gainScheduleData gainSchedule;

    PID control_pid = PID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

    bool running;

    bool calibrationRunning;
    float calibrationProgress = 0; // fraction between 0 and 1

    bool initialised;
    bool sensorInitialised;
    bool dataInitialised;

    enum calibrationStates
    {
        ground,
        pumping,
        leaking,
    };

    calibrationStates calibrationState;

    float calibrationSetPointPressure;

    bool sdInitialised;

    bool gainScheduleInitialised;

    float currentSeconds;

    unsigned long startMillis;

    Sd sd;

    unsigned long timePassed;  // microseconds
    unsigned long lastLogTime; // microseconds
    unsigned long logTime;     // microseconds
};

#endif // CONTROLLER_H