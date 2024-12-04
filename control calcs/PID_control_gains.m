function [Kp, Ki, Kd] = PID_control_gains(Qpump_at_P_0, k_leak, P_0, P_atm, scale_factor, desired_bandwidth)
    % Ensuring signs are consistent
    k_leak = abs(k_leak);
    P_diff = P_atm - P_0; % Should be positive if P_atm > P_0

    % Calculate leak rate at P_0
    Qleak_at_P_0 = abs(k_leak * P_diff); % Positive value
    % I think we need to add a constant offset value

    Qpump_at_P_0 = abs(Qpump_at_P_0);

    % Calculate the actual pump rate by subtracting the leak rate
    Qpump_actual = Qpump_at_P_0 + Qleak_at_P_0; % Negative value (pressure decreasing)

    % Ensure that Qpump_actual is negative
    Qpump_actual = -abs(Qpump_actual);

    % Define transfer function parameters
    A = - k_leak;   %                   !!! Should this be negative or positive?
    B = Qpump_actual; % Negative value

    A = A * scale_factor; % Scaled A
    B = B / scale_factor; % Scaled B

    % disp(A);
    % disp(B);

    % Define the plant transfer function
    s = tf('s');
    G = B / (s + A);

    % Tune the PID controller
    C = pidtune(G, 'PID', desired_bandwidth);

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
end