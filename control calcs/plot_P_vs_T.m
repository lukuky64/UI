% Read the data from CSV

filename2 = 'calibration files/CAL_9_a=0.5.csv';
filename1 = 'calibration files/CAL_8_a=0.5.csv';
 
data1 = readtable(filename1);
data2 = readtable(filename2);
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


pressure1 = data1.pressure;
time1 = data1.time;

pressure2 = data2.pressure;
time2 = data2.time;

figure;

subplot(3, 1, 1);
plot(time1, pressure1, '.-', Color='red');
title(filename1)

subplot(3, 1, 2);
plot(time2, pressure2, '.-', Color='blue');
title(filename2)

subplot(3, 1, 3);
plot(time1, pressure1, '.-', Color='red');
hold on;
plot(time2, pressure2, '.-', Color='blue');
hold off;


% Run the create_data function
[Qpumping, Qleaking, ln_dP_leak, P_atm] = create_data(filename1);



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