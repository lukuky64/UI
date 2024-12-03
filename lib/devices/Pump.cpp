#include "Pump.h"

Pump::Pump() : minSpeed(0), maxSpeed(255), pwmPin(MOTOR_PWM), dirPin(MOTOR_DIR)
{
    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    sendCommand(0); // default to off
}

// speed is a percentage
void Pump::sendCommand(int8_t speed)
{
    if (speed >= 0)
    {
        command.direction = BLOW;
    }
    else
    {
        command.direction = SUCK;
        speed = -speed;
    }

    speed = constrain(speed, 0, 100);
    command.speed = map(speed, 0, 100, minSpeed, maxSpeed);

    digitalWrite(dirPin, command.direction); // need to find which direction is suck and blow
    analogWrite(pwmPin, command.speed);      // range is 0-255
}

PumpData Pump::getCommand()
{
    return command;
}