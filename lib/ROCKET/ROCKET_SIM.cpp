#include "ROCKET_SIM.h"

ROCKET_SIM::ROCKET_SIM(float burnout_time, float apogee, float terminal_velocity)
{
    this->burnout_time = burnout_time;
    this->apogee = apogee;
    this->terminal_velocity = terminal_velocity;
}

sim_data &ROCKET_SIM::runSimulation()
{
    // DBG("Initialised constants: " + String(this->burnout_time) + ", " + String(this->apogee) + ", " + String(this->terminal_velocity));

    float acceleration = compute_a_b(this->gravity, this->burnout_time, this->apogee);

    // DBG("Acceleration: " + String(acceleration));

    float time_apogee = abs((this->burnout_time * (acceleration - this->gravity)) / (-this->gravity));

    // DBG("Time to apogee: " + String(time_apogee));

    // time and distance from apogee to terminal velocity
    float time_a_t = abs(this->terminal_velocity / this->gravity);
    float dist_t = this->apogee + ((pow(this->terminal_velocity, 2)) / (2 * this->gravity));

    // time from terminal to landing
    float time_t_l = abs(dist_t / terminal_velocity);

    float time_total = time_apogee + time_a_t + time_t_l;

    // DBG("Total time: " + String(time_total));

    if (time_total <= 0)
    {
        return this->data;
    }

    for (int i = 0; i < resolution; i++)
    {
        this->data.altitude[i] = 0.0f;
        this->data.velocity[i] = 0.0f;
        this->data.pressure[i] = 0.0f;
    }

    float prev_t = 0.0;
    float dt = 0;

    // Previous graph point
    int prev_x = 0;
    int prev_y = 0;

    for (int x = 1; x < resolution; x++)
    {
        this->data.time[x] = mapFloat(x, 1.0f, float(resolution - 1), 0.0f, time_total);
        dt = this->data.time[x] - prev_t;

        if (this->data.time[x] >= this->burnout_time)
        {
            if (this->data.velocity[x - 1] <= this->terminal_velocity)
            {
                acceleration = 0;
                this->data.velocity[x - 1] = this->terminal_velocity;
            }
            else
            {
                acceleration = this->gravity;
            }
        }

        // Update velocity and altitude using kinematic equations
        this->data.altitude[x] = this->data.altitude[x - 1] + (this->data.velocity[x - 1] * dt) + (0.5 * acceleration * pow(dt, 2));
        this->data.pressure[x] = altitudeToPressure(this->data.altitude[x]);
        this->data.velocity[x] = this->data.velocity[x - 1] + acceleration * dt;

        // Update previous points
        prev_x = x;
        prev_y = this->data.altitude[x];
        prev_t = this->data.time[x];

        if (this->data.altitude[x] < 1.0 && this->data.time[x] > time_apogee)
        {
            this->data.num_points = x;
            break;
        }
    }

    // Find the maximum altitude and its index
    float *max_ptr = std::max_element(this->data.altitude, this->data.altitude + this->data.num_points);
    int max_index = static_cast<int>(max_ptr - this->data.altitude);

    // DBG("Max index: " + String(max_index));

    // Retrieve the maximum altitude and corresponding time
    this->data.apogee = this->data.altitude[max_index];

    // DBG("Apogee: " + String(this->data.apogee) + "m");

    this->data.time_at_apogee = this->data.time[max_index];

    return this->data;
}

float ROCKET_SIM::compute_a_b(double g, double t_b, double S_a)
{
    // Calculate -g * t_b^2
    double term1 = -g * t_b * t_b;

    // Calculate the square root argument: (term1)^2 - 4 * t_b^2 * 2gS_a
    double sqrt_argument = (term1 * term1) - (4.0 * t_b * t_b * 2.0 * g * S_a);

    // Check if the sqrt argument is non-negative
    if (sqrt_argument < 0.0)
    {
        // Serial.println("Error: Negative argument inside sqrt. Returning NaN.");
        return NAN; // Not a Number to indicate an error
    }

    // Compute the square root
    double sqrt_value = sqrt(sqrt_argument);

    // Compute the numerator: g * t_b^2 + sqrt(g^2 * t_b^4 - 8gS_a * t_b^2)
    double numerator = g * t_b * t_b + sqrt_value;

    // Compute the denominator: 2 * t_b^2
    double denominator = 2.0 * t_b * t_b;

    // Compute y
    double y = numerator / denominator;

    return y;
}

float ROCKET_SIM::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float ROCKET_SIM::altitudeToPressure(float h)
{
    // Calculate the base of the exponent
    float base = 1 + ((temp_lapse_rate / temperature) * (h - h_b));

    // Calculate the exponent
    float exponent = (gravity * molar_mass) / (gas_constant * temp_lapse_rate);

    // Calculate final pressure using the formula
    float P = static_pressure * pow(base, exponent);

    return P;
}

float ROCKET_SIM::pressureToAltitude(float P)
{
    // Calculate the exponent for (P / static_pressure) based on rearranged formula
    float exponent = (-gas_constant * temp_lapse_rate) / ((-gravity) * molar_mass);

    // Calculate the term inside the parentheses
    float term = pow(P / static_pressure, exponent) - 1.0f;

    // Calculate altitude based on the rearranged formula
    float h = h_b + (temperature / temp_lapse_rate) * term;

    return h;
}