#ifndef GAIN_SCHEDULE_DATA_H
#define GAIN_SCHEDULE_DATA_H

#include "Arduino.h"

#define MAX_GAIN_ROWS 100

struct gainScheduleData
{
    uint8_t height;                    // Number of rows read from the file
    static const uint8_t width = 4;    // Number of columns (fixed at 4)
    double data[MAX_GAIN_ROWS][width]; // array to store the data. [P, I, D, pressure]
};

#endif // GAIN_SCHEDULE_DATA_H
