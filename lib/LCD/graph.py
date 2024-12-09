import matplotlib.pyplot as plt
import math

def solve_for_acceleration(apogee, burnout_time, apogee_time, gravity):
    """
    Calculate the required acceleration to reach apogee within burnout_time.

    Parameters:
    - apogee (float): The maximum altitude in meters.
    - burnout_time (float): The time at which burnout occurs in seconds.
    - apogee_time (float): The time at which apogee is reached in seconds.
    - gravity (float): The acceleration due to gravity (negative value) in m/s².

    Returns:
    - float: The calculated acceleration in m/s².
    """
    numerator = apogee - (0.5 * gravity * apogee_time ** 2)
    denominator = ((burnout_time ** 2) / 2.0) + (burnout_time * apogee_time)
    acceleration = numerator / denominator
    return acceleration

def calculate_trajectory(apogee, finish_time):
    """
    Calculate the altitude over time based on the provided parameters.

    Parameters:
    - apogee (float): The maximum altitude in meters.
    - finish_time (float): The total time of the trajectory in seconds.

    Returns:
    - tuple: Two lists containing time and corresponding altitude values.
    """
    # Define time segments
    burnout_time = finish_time * 0.05
    apogee_time = finish_time * 0.25

    # Gravity in m/s² (assuming downward is negative)
    gravity = -9.81

    # Calculate required acceleration to reach apogee within burnout_time
    acceleration = solve_for_acceleration(apogee, burnout_time, apogee_time, gravity)

    # Terminal velocity on descent (m/s)
    terminal_velocity = -10.0

    # Calculate time to reach terminal velocity
    time_to_terminal = 0.75 * finish_time - (terminal_velocity / (2.0 * gravity))

    # Initialize velocity and altitude
    velocity = 0.0
    altitude = 0.0
    prev_time = 0.0

    # Number of points for the graph (similar to SCREEN_WIDTH in Arduino)
    num_points = 320
    time_values = []
    altitude_values = []

    for x in range(1, num_points + 1):
        # Map screen x to time
        current_time = (x / num_points) * finish_time
        dt = current_time - prev_time

        # Determine current acceleration
        if current_time > burnout_time:
            current_acceleration = gravity
        else:
            current_acceleration = acceleration

        if current_time > time_to_terminal:
            current_acceleration = 0.0
            velocity = terminal_velocity  # Set to terminal velocity

        # Update velocity and altitude using kinematic equations
        altitude += velocity * dt + 0.5 * current_acceleration * dt ** 2
        velocity += current_acceleration * dt

        # Append current time and altitude to lists
        time_values.append(current_time)
        altitude_values.append(altitude)

        # Update previous time
        prev_time = current_time

    return time_values, altitude_values, burnout_time, time_to_terminal

def plot_trajectory(time, altitude, apogee, burnout_time, time_to_terminal):
    """
    Plot the altitude over time using matplotlib.

    Parameters:
    - time (list): List of time values in seconds.
    - altitude (list): List of altitude values in meters.
    - apogee (float): The maximum altitude in meters.
    - burnout_time (float): The time at which burnout occurs in seconds.
    - time_to_terminal (float): The time at which terminal velocity is reached in seconds.
    """
    plt.figure(figsize=(12, 6))
    plt.plot(time, altitude, label='Altitude')

    # Plot apogee line
    plt.axhline(y=apogee, color='magenta', linestyle='--', label='Apogee')

    # Plot burnout time
    plt.axvline(x=burnout_time, color='red', linestyle='--', label='Burnout Time')

    # Plot time to terminal velocity
    plt.axvline(x=time_to_terminal, color='green', linestyle='--', label='Time to Terminal Velocity')

    plt.title('Rocket Trajectory')
    plt.xlabel('Time (s)')
    plt.ylabel('Altitude (m)')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def main():
    # Example parameters
    apogee = 600.0       # in meters
    finish_time = 10.0    # in seconds

    # Calculate trajectory
    time, altitude, burnout_time, time_to_terminal = calculate_trajectory(apogee, finish_time)

    # Plot the trajectory
    plot_trajectory(time, altitude, apogee, burnout_time, time_to_terminal)

if __name__ == "__main__":
    main()