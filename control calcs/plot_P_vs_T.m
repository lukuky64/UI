% Read the data from CSV

filename = 'cal_11_12_1.csv';

% filename = 'CAL_30.csv';
%filename = 'vacuum_chamber_dynamics.csv';
 
data = readtable(filename);
%  
% Separate data by states
% data_initial = data(data.state == 0, :);
% data_pumping = data(data.state == 1, :);
% data_leaking = data(data.state == 2, :);
% 
% totalRowsPump = height(data_pumping);
% totalRowsLeak = height(data_leaking);
% 
% rowsToTakePump = floor(totalRowsPump * 0.75);
% rowsToTakeLeak = floor(totalRowsLeak * 0.75);

% 
% data_pumping = data_pumping(1:rowsToTakePump, :);
% data_leaking = data_leaking((totalRowsLeak - rowsToTakeLeak):totalRowsLeak, :);


pressure = data.pressure;
time = data.time;

figure;
plot(time, pressure, '.-');



% Run the create_data function
[Qpumping, Qleaking, ln_dP_leak, P_atm] = create_data(filename);



figure;

% Subplot 1: Pumping Rate vs Operating Pressure
subplot(2, 1, 1); % 2 rows, 1 column, 1st plot
plot(Qpumping(:, 2), Qpumping(:, 1), '.-');
xlabel('Operating Pressure (Pa)');
ylabel('Pumping Rate (Pa/s)');
title('Pumping Rate vs Operating Pressure');
grid on;

% Subplot 2: Leaking Rate vs Operating Pressure
subplot(2, 1, 2); % 2 rows, 1 column, 2nd plot
plot(Qleaking(:, 2), Qleaking(:, 1), '.-');
xlabel('Operating Pressure (Pa)');
ylabel('Leaking Rate (Pa/s)');
title('Leaking Rate vs Operating Pressure');
grid on;