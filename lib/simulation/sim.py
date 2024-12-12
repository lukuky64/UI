import numpy as np
import matplotlib.pyplot as plt

# Constants
V = 0.1  # Chamber volume (m³)
Q_max = 0.01  # Maximum pump rate (m³/s)
L = 1e-5  # Leak rate (m³/s)
dt = 0.1  # Time step (s)
time = np.arange(0, 100, dt)

# Pump efficiency function
def f(P):
    k = 0.01  # Fitting parameter
    return 1 - np.exp(-k * P)

# PID controller
def pid_control(error, prev_error, integral, Kp, Ki, Kd, dt):
    integral += error * dt
    derivative = (error - prev_error) / dt
    control = Kp * error + Ki * integral + Kd * derivative
    return control, integral

# Simulation
P = [1000]  # Initial pressure (Pa)
target_pressure = 100  # Target pressure (Pa)
Kp, Ki, Kd = 0.1, 0.01, 0.5  # PID gains
integral, prev_error = 0, 0

for t in time[1:]:
    error = target_pressure - P[-1]
    control, integral = pid_control(error, prev_error, integral, Kp, Ki, Kd, dt)
    prev_error = error

    # Pump response
    Q_pump = Q_max * f(P[-1]) * control
    dP = -Q_pump / V + L
    P.append(P[-1] + dP * dt)

# Plot results
plt.plot(time, P)
plt.axhline(target_pressure, color='r', linestyle='--', label='Target Pressure')
plt.xlabel('Time (s)')
plt.ylabel('Pressure (Pa)')
plt.legend()
plt.show()