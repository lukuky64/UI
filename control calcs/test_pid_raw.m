scale_factor = 100;


A = 0.02;    % Example value for A (k_leak)
B = - 200;    % Example value for B (K_m)

A = A * scale_factor % Scaled A
B = B / scale_factor % Scaled B

% Define the plant transfer function
s = tf('s');
G = B / (s + A);

% Tune the PID controller
C = pidtune(G, 'PID');

% Assign gains to outputs
Kp = C.Kp
Ki = C.Ki
Kd = C.Kd