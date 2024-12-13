#include "Pump.h"

Pump::Pump() : minSpeed(0), maxSpeed(255), pwmPin(MOTOR_PWM), dirPin(MOTOR_DIR)
{
    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    sendCommand(0); // default to off
}

// speed is a percentage, -100 to 100
void Pump::sendCommand(double speed)
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

    speed = (uint8_t)round(constrain(speed, 0, 100));       // make sure it's in the range 0-100 and an integer
    command.speed = map(speed, 0, 100, minSpeed, maxSpeed); // map to 0-255

    digitalWrite(dirPin, command.direction); // need to find which direction is suck and blow
    analogWrite(pwmPin, command.speed);      // range is 0-255
}

PumpCommand Pump::getCommand()
{
    return command;
}