#ifndef ROCKET_SIM_H
#define ROCKET_SIM_H

#include <Arduino.h>
#include "DataType.h"
#include "Debug.hpp"

class ROCKET_SIM
{
public:
    ROCKET_SIM(float burnout_time, float apogee, float terminal_velocity);

    static float altitudeToPressure(float h);
    static float pressureToAltitude(float P);

    sim_data &runSimulation();

private:
    float compute_a_b(double g, double t_b, double S_a);
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

    // constants
    static constexpr float gravity = -9.81;
    static constexpr float temperature = 300.0;        // temp at base, degrees K, 300K = 27C
    static constexpr float temp_lapse_rate = -0.0065;  // K/m
    static constexpr float static_pressure = 101325.0; // sea level, Pa
    static constexpr float gas_constant = 8.31432;     // Nm/molK
    static constexpr float molar_mass = 0.0289644;     // kg/mol
    static constexpr float h_b = 0.0;                  // base altitude, m

    float burnout_time;
    float apogee;
    float terminal_velocity;

    sim_data data;
};

#endif // ROCKET_SIM_H