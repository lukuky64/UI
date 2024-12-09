% Example MATLAB script to run the rocket simulation and plot altitude vs time

close all; clear; clc;

% ---------------------------
% Define constants
% ---------------------------
gravity = -9.81;             % m/s²
burnout_time = 0.5;           % Example burnout time in seconds
apogee = 1000;               % Example desired apogee in metres
terminal_velocity = -8;     % Example terminal velocity in m/s
resolution = 320 * 50;          % Number of data points

% Atmospheric and gas constants
temp_lapse_rate = -0.0065;  % K/m (typical environmental lapse rate)
temperature = 300;       % K (approx. 15°C at sea level)
h_b = 0;                    % Base altitude in metres
static_pressure = 101325;   % Pa (standard sea level pressure)
molar_mass = 0.0289644;     % kg/mol (molar mass of Earth's air)
gas_constant = 8.3144598;   % J/(mol*K) (universal gas constant)

% Run the simulation
data = runSimulation(burnout_time, apogee, terminal_velocity, gravity, ...
                     resolution, temp_lapse_rate, temperature, h_b, ...
                     static_pressure, molar_mass, gas_constant);

% Plot the altitude vs. time
figure;
plot(data.time, data.altitude, 'LineWidth', 1.5);
xlabel('Time (s)');
ylabel('Altitude (m)');
title('Altitude vs. Time');
grid on;

% ---------------------------
% Functions
% ---------------------------

function y = compute_a_b(g, t_b, S_a)
    % Translated from C++ logic to MATLAB
    term1 = -g * t_b^2;
    sqrt_argument = (term1 * term1) - (4.0 * t_b^2 * 2.0 * g * S_a)

    if sqrt_argument < 0.0
        y = NaN;
        return;
    end

    sqrt_value = sqrt(sqrt_argument);
    numerator = g * t_b^2 + sqrt_value;
    denominator = 2.0 * t_b^2;
    y = numerator / denominator;
end

function P = altitudeToPressure(h, temp_lapse_rate, temperature, h_b, gravity, molar_mass, gas_constant, static_pressure)
    base = 1 + ((temp_lapse_rate / temperature) * (h - h_b));
    exponent = (gravity * molar_mass) / (gas_constant * temp_lapse_rate);
    P = static_pressure * (base^exponent);
end

function data = runSimulation(burnout_time, apogee, terminal_velocity, gravity, resolution, ...
                              temp_lapse_rate, temperature, h_b, static_pressure, molar_mass, gas_constant)
    % Compute initial acceleration
    acceleration = compute_a_b(gravity, burnout_time, apogee)

    % Compute key times
    time_apogee = abs((burnout_time * (acceleration - gravity)) / (-gravity));
    time_a_t = abs(terminal_velocity / gravity);
    dist_t = apogee + ((terminal_velocity^2) / (2 * gravity));
    time_t_l = abs(dist_t / terminal_velocity);
    time_total = time_apogee + time_a_t + time_t_l;

    if time_total <= 0
        data = [];
        return;
    end

    % Preallocate arrays
    data.velocity = zeros(1, resolution);
    data.altitude = zeros(1, resolution);
    data.time = zeros(1, resolution);
    data.pressure = zeros(1, resolution);

    % Initial conditions
    data.velocity(1) = 0.0;
    data.altitude(1) = 0.0;
    data.time(1) = 0.0;

    prev_t = 0.0;
    acc_current = acceleration;

    % Simulation loop
    for i = 2:resolution
        % Map index to time linearly from 0 to time_total
        data.time(i) = ((i - 2) / (resolution - 2)) * time_total;
        dt = data.time(i) - prev_t;

        % After burnout, adjust acceleration and velocity if terminal velocity is reached
        if data.time(i) >= burnout_time
            if data.velocity(i - 1) <= terminal_velocity
                acc_current = 0;
                data.velocity(i - 1) = terminal_velocity;
            else
                acc_current = gravity;
            end
        end

        % Update altitude, pressure, velocity
        data.altitude(i) = data.altitude(i - 1) + data.velocity(i - 1)*dt + 0.5*acc_current*(dt^2);
        data.pressure(i) = altitudeToPressure(data.altitude(i), temp_lapse_rate, temperature, ...
                                              h_b, gravity, molar_mass, gas_constant, static_pressure);
        data.velocity(i) = data.velocity(i - 1) + acc_current*dt;

        prev_t = data.time(i);

        % Stop if near ground level after apogee
        if data.altitude(i) < 1.0 && data.time(i) > time_apogee
            data.num_points = i;
            break;
        end
    end

    % Find apogee
    [max_alt, max_idx] = max(data.altitude);
    data.apogee = max_alt;
    data.time_at_apogee = data.time(max_idx);
end