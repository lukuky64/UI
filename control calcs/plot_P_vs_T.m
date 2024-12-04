% Read the data from CSV

filename = 'CAL_30.csv';
% data = readtable(filename);
% % 
% % Separate data by states
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
% 
% data_pumping = data_pumping(1:rowsToTakePump, :);
% data_leaking = data_leaking((totalRowsLeak - rowsToTakeLeak):totalRowsLeak, :);
% 
% 
% pressure = data_leaking.pressure;
% time = data_leaking.time;
% 
% figure;
% plot(time, pressure, '-');


% Run the create_data function
[Qpumping, Qleaking, ln_dP_leak, P_atm] = create_data(filename);