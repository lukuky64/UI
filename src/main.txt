#include "LCD.h"
#include <Arduino.h>
#include "Debug.hpp"

LCD lcd;

void setup(void)
{
    INITIALISE_DBG(115200);
    lcd.begin();
}

void loop(void)
{
    lcd.command();
}
