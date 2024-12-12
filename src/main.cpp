#include "UI.h"
#include <Arduino.h>
#include "Debug.hpp"

UI *ui;

void setup(void)
{
    INITIALISE_DBG(115200);
    delay(500);

    ui = new UI();
    ui->begin();
}

void loop(void)
{
    // DBG("test");
    ui->command();
}
