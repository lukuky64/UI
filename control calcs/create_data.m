function [Qpumping, Qleaking, ln_dP_leak, P_atm] = create_data(filename)
    % calibration_data.csv

    % expected data structure from csv: 
    % state | time | pressure
    
    % If the .csv file contains mixed data types (e.g., numbers and text):
    data = readtable(filename);
    
    % seperate data by states
    data_initial = data(data.state == 0, :);
    data_pumping = data(data.state == 1, :);
    data_leaking = data(data.state == 2, :);


    % pumping_time = data.time;
    % dt_pumping = mean(diff(pumping_time));
    % 
    % Fs = 1/dt_pumping; % sampling frequnecy
    % cutoff_frequnecy_Hz = 0.5;
    % 
    % data_pumping.pressure = lowpass(data_pumping.pressure, cutoff_frequnecy_Hz, Fs);
    % data_leaking.pressure = lowpass(data_leaking.pressure, cutoff_frequnecy_Hz, Fs);

    % order = 3;
    % framelen = 37;
    % data_pumping.pressure = sgolayfilt(data_pumping.pressure,order,framelen);
    % data_leaking.pressure = sgolayfilt(data_leaking.pressure,order,framelen);



    totalRowsPump = height(data_pumping);
    totalRowsLeak = height(data_leaking);

    data_pumping = data_pumping(floor(totalRowsPump * 0.1):floor(totalRowsPump * 0.8), :);
    data_leaking = data_leaking((floor(totalRowsLeak * 0.2)):floor(totalRowsLeak * 0.9), :);

    
    if (height(data_initial) >= 1)
        P_atm = median(table2array(data_initial(:, 3))); % get the median atmospheric pressure, excluding outliers
        P_atm = double(P_atm);
    else
        P_atm = 101325; % if no data available, default to standard
    end
    
    
    % Guard for minimum rows
    if height(data_pumping) < 2 || height(data_leaking) < 2
        error('Not enough data points to compute rates.');
    end
    
    % [pumping rate at max speed (Pa/s), operating pressure (Pa)]
    Qpumping = zeros((height(data_pumping) - 1), 2); 

    Qleaking = zeros((height(data_leaking) - 1), 2); 
    
    % [leaking rate, no pumping (Pa/s), operating pressure (Pa)]
    ln_dP_leak = zeros((height(data_leaking) - 1), 2); 
    
    
    for i=1:((height(data_pumping)) - 1)
        Qpumping(i, 1) = (data_pumping.pressure(i + 1) - data_pumping.pressure(i)) / (data_pumping.time(i + 1) - data_pumping.time(i));
        
        % disp(['First = ' num2str(data_pumping.time(i + 1))]);

            
        % disp((data_pumping.time(i + 1) - data_pumping.time(i)));

        Qpumping(i, 2) = data_pumping.pressure(i); % operating point
    end
    
    
    for i=1:((height(data_leaking)) - 1)
        ln_dP_leak(i, 1) = log(data_leaking.pressure(i + 1) - data_leaking.pressure(i));
        ln_dP_leak(i, 2) = data_leaking.time(i); % time data


        Qleaking(i, 1) = (data_leaking.pressure(i + 1) - data_leaking.pressure(i)) / (data_leaking.time(i + 1) - data_leaking.time(i));
        Qleaking(i, 2) = data_leaking.pressure(i); % operating point
    end
    


% pumping_time = data.time;
% dt_pumping = mean(diff(pumping_time));
% 
% Fs = 1/dt_pumping; % sampling frequnecy
% cutoff_frequnecy_Hz = 1;
% 
% 
% order = 5;
% framelen = 19;
% Qleaking(:, 1) = sgolayfilt(Qleaking(:, 1),order,framelen);
% Qpumping(:, 1) = sgolayfilt(Qpumping(:, 1),order,framelen);


% Qleaking(:, 1) = lowpass(Qleaking(:, 1),cutoff_frequnecy_Hz,Fs);
% Qpumping(:, 1) = lowpass(Qpumping(:, 1),cutoff_frequnecy_Hz,Fs);




% Create a figure window
% figure;
% 
% % Subplot 1: Pumping Rate vs Operating Pressure
% subplot(2, 1, 1); % 2 rows, 1 column, 1st plot
% plot(Qpumping(:, 2), Qpumping(:, 1), '-');
% xlabel('Operating Pressure (Pa)');
% ylabel('Pumping Rate (Pa/s)');
% title('Pumping Rate vs Operating Pressure');
% grid on;
% 
% % Subplot 2: Leaking Rate vs Operating Pressure
% subplot(2, 1, 2); % 2 rows, 1 column, 2nd plot
% plot(Qleaking(:, 2), Qleaking(:, 1), '-');
% xlabel('Operating Pressure (Pa)');
% ylabel('Leaking Rate (Pa/s)');
% title('Leaking Rate vs Operating Pressure');
% grid on;

end