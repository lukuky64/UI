#include "UI.h"
#include <Arduino.h>
#include "Debug.hpp"

UI ui;

void setup(void)
{
    INITIALISE_DBG(115200);
    ui.begin();
}

void loop(void)
{
    ui.command();
}
