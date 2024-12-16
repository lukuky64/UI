#include "PressureSensor.h"

PressureSensor::PressureSensor() : basePressure(0), ADC_RES(12)
{
    // from datasheet: 0psi = 0.5V, 75psi = 2.5V, 150psi = 4.5V
    scaleFactor = 75.0 / (2.5 - 0.5);
    scaleFactor *= 6894.76; // convert psi to Pa
}

bool PressureSensor::begin(u_int8_t sensorPin_)
{
    sensorType = analog;
    // ensure sensorPin is an analog pin
    sensorPin = sensorPin_;
    pinMode(sensorPin, INPUT);
    delay(50);

    int numReadingsAvg = 20; // this will take 1 second to calibrate
    float avgPressure = 0;

    for (int i = 0; i < numReadingsAvg; i++)
    {
        avgPressure += getPressure(true);
        delay(50);
    }

    basePressure = avgPressure / numReadingsAvg;
    // DBG(basePressure);
    return true;
}

bool PressureSensor::begin(uint8_t SDA_, uint8_t SCL_, uint8_t addr_)
{
    sensorType = BMP280;

    wire.begin(SDA_, SCL_);
    bmp = Adafruit_BMP280(&wire);
    addr = addr_;

    if (!bmp.begin(addr))
    {
        return false;
    }

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,   /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_NONE, /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X8,   /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X8,     /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    delay(20);

    int numReadingsAvg = 20; // this will take 1 second to calibrate

    float avgPressure = 0;

    for (int i = 0; i < numReadingsAvg; i++)
    {
        avgPressure += getPressure(false);
        delay(50);
    }

    basePressure = avgPressure / numReadingsAvg;
    // DBG(basePressure);

    return true;
}

float PressureSensor::getBasePressure()
{
    return basePressure;
}

bool PressureSensor::testConnection()
{
    if (sensorType == BMP280)
    {
        uint8_t status = bmp.getStatus();

        if (status == 243) // BMP280_REGISTER_STATUS, apparently this is what happens when we have error
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else if (sensorType == analog)
    {
        return true;
    }
    return false;
}

float PressureSensor::getPressure(bool absolute)
{
    if (sensorType == BMP280)
    {
        if (bmp.takeForcedMeasurement())
        {
            pressure = (bmp.readPressure()) - basePressure; // subtract atmospheric pressure

            if (absolute)
            {
                pressure += 101325; // convert to absolute pressure
                // DBG(pressure);
            }
        }
        else
        {
            return -1;
        }
    }
    else if (sensorType == analog)
    {
        analogReadResolution(ADC_RES);
        uint16_t rawReading = analogRead(sensorPin);
        analogReadResolution(10);                                    // need to switch back to 10-bit for the LCD display :(
        float voltage = (rawReading * 3.3f) / (float)(1 << ADC_RES); // bit shifting is faster than power function
        // DBG(voltage);
        pressure = ((voltage - 0.5) * scaleFactor) - basePressure; // -0.5V offset from sensor datasheet

        if (absolute)
        {
            pressure += 101325; // convert to absolute pressure
            // DBG(pressure);
        }
    }

    return pressure; // units should be in Pa
}
