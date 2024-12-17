#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "Debug.hpp"

class PressureSensor
{

public:
    PressureSensor();
    bool begin(uint8_t sensorPin_);
    bool begin(uint8_t SDA_, uint8_t SCL_, uint8_t addr_);

    float getPressure(bool absolute);
    bool testConnection();

    float getBasePressure();

    void calibrateBasePressure();

private:
    float pressure;
    float basePressure; // might be useful for calibration

    int sensorPin;
    float scaleFactor;

    enum sensorTypes
    {
        analog,
        BMP280
    };

    sensorTypes sensorType;

    Adafruit_BMP280 bmp;
    sensors_event_t pressure_event;
    Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

    TwoWire wire;

    uint8_t addr;

    int ADC_RES;
};