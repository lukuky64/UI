% Vacuum Chamber Dynamics Simulation Script with Plot

% Parameters
A = 0.02;    % Example value for A (k_leak)   !!! 0.02
B = -200.0;    % Example value for B (K_m)
P_0 = 101325;  % Initial pressure in the chamber (Pa)
dt = 0.1;      % Time step (s)
t_end = 150;   % Simulation end time (s)
noise_std = 0.5; % 0.2; %3.5; % from bmp280 datasheet -> variance = 12Pa

% Initialize variables
time = 0:dt:t_end;     % Time vector
P = zeros(size(time)); % Pressure vector
state = zeros(size(time)); % State vector to indicate pump status
P(1) = P_0;            % Initial pressure

% Simulate dynamics
for i = 2:length(time)
    
    if (i > (length(time)/8))
        u = 0;           % Pump off
        state(i) = 2;    % State indicator: pump off
    else
        u = 1;           % Pump on
        state(i) = 1;    % State indicator: pump on
    end

    % Update pressure using Euler integration
    dP_dt = A * (P_0 - P(i-1)) + B * u; % Corrected pressure rate of change
    P(i) = P(i-1) + dP_dt * dt;  % Update pressure
    
    % Ensure pressure doesn't go below zero
    P(i) = max(0, P(i));
end


% add relative noise to each value
for i = 2:length(time)
    P(i) = P(i) + (noise_std * randn);
end



% Combine results into a matrix for saving
results = [state', time', P'];

% Plot the pressure profile
figure;
plot(time, P, 'LineWidth', 2);
grid on;
title('Vacuum Chamber Pressure Profile');
xlabel('Time (s)');
ylabel('Pressure (Pa)');
legend('Chamber Pressure', 'Location', 'best');

% Save results as a CSV file
fileName = 'vacuum_chamber_dynamics.csv';
headers = {'state', 'time', 'pressure'};
dataWithHeaders = [headers; num2cell(results)];
writecell(dataWithHeaders, fileName);