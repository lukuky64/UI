function [Kp, Ki, Kd] = PID_control_gains(Qpump_at_P_0, Qleak_at_P_0, P_atm, P0, scale_factor, desired_bandwidth)
    
    Qleak_at_P_0 = abs(Qleak_at_P_0);
    Qpump_at_P_0 = abs(Qpump_at_P_0);

    k_leak = Qleak_at_P_0 / (P_atm - P0);  % s^-1

    % Calculate the actual pump rate by subtracting the leak rate
    Qpump_actual = Qpump_at_P_0 + Qleak_at_P_0; % Negative value (pressure decreasing)

    % Ensure that Qpump_actual is negative
    Qpump_actual = -abs(Qpump_actual);

    % Define transfer function parameters
    A = k_leak;   %                   !!! Should this be negative or positive?
    B = Qpump_actual; % Negative value

    A = A * scale_factor; % Scaled A
    B = B / scale_factor; % Scaled B

    % disp(A);
    % disp(B);

    % Define the plant transfer function
    s = tf('s');
    G = B / (s + A);

    % Tune the PID controller

    C = pidtune(G, 'PID'); % !!! not using desired_bandwidth ATM

    % Assign gains to outputs
    Kp = C.Kp;
    Ki = C.Ki;
    Kd = C.Kd;

    % Optional: Simulate the closed-loop response
    % T = feedback(C * G, 1);
    % figure;
    % step(T);
    % grid on;
    % title('Closed-Loop Step Response');

    % Display pole locations for verification
    % pole_locations = pole(G);
    % disp('Pole Locations:');
    % disp(pole_locations);

end