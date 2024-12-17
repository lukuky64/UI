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
    bool initData(sim_data &data_);
    bool initDevices(float alpha_ = 0.5);
    bool run();
    void stop();
    bool iterate();
    float getLatestTime();
    float getLatestPressure();
    float getLatestSetpoint();
    bool calibrateSystem(float setPoint);
    bool initCalibrateSystem(float setPoint);
    bool LogDesiredData(String stateData, bool forceLog);
    bool startCalibrateSystem();
    bool calibrateIterate();
    bool updateReading();
    bool initSensor(float alpha_ = 0.5);
    float getCalibrationProgress();
    void setCalibrationProgress(float calibrationProgress_);

    void initPID();

    bool updateGains();
    bool initGainSchedule();
    void setAlpha(float alpha_);
    float getAlpha();

private:
    Pump pump;
    PressureSensor pressureSensor;
    sim_data *data = nullptr; // Pointer to sim_data

    float filteredReading; // initial guess of sea level pressure
    float alpha;           // high alpha means more weight to new data

    // PID control
    double Setpoint, Input, Output = 0;
    double Kp = 0.01;
    double Ki = 0;
    double Kd = 0;

    uint32_t safePressureLow = 26436;   // -100m in Pa
    uint32_t safePressureHigh = 102532; // 10,000m in Pa

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
    unsigned long logFreq;     // microseconds
};

#endif // CONTROLLER_H