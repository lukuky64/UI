#ifndef PUMP_H
#define PUMP_H

#include "Arduino.h"
#include "Debug.hpp"

struct PumpCommand
{
    uint8_t speed;
    uint8_t direction;
};

class Pump
{
public:
    Pump();
    void sendCommand(double speed);
    PumpCommand getCommand();

private:
    int maxSpeed;
    int minSpeed;

    PumpCommand command;

    int pwmPin;
    int dirPin;

    // need to find which direction is what
    const uint8_t SUCK = LOW;
    const uint8_t BLOW = HIGH;
};

#endif // PUMP_H