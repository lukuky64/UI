#ifndef DATATYPE_H
#define DATATYPE_H

static const int resolution = 8 * 320;

struct sim_data
{
    float velocity[resolution];
    float altitude[resolution];
    float pressure[resolution];
    float time[resolution];
    float apogee = 0;
    float time_at_apogee = 0;
    int num_points = 0;
};

#endif