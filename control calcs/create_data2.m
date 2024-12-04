function [Qpumping, Qleaking, ln_dP_leak, P_atm] = create_data2(filename)
    % create_data.m
    % Processes vacuum chamber data and computes the first derivative using Savitzky-Golay filters.

    % Read the CSV file into a table
    data = readtable(filename);

    % Validate expected columns
    expectedCols = {'state', 'time', 'pressure'};
    if ~all(ismember(expectedCols, data.Properties.VariableNames))
        error('Input CSV must contain columns: state, time, pressure');
    end

    % Separate data by states
    data_initial = data(data.state == 0, :);
    data_pumping = data(data.state == 1, :);
    data_leaking = data(data.state == 2, :);

    % Remove the transitional data between states
    totalRowsPump = height(data_pumping);
    totalRowsLeak = height(data_leaking);

    % Define slicing fractions
    pump_start_frac = 0.1;
    pump_end_frac = 0.8;
    leak_start_frac = 0.2;
    leak_end_frac = 0.9;

    % Calculate slicing indices with safeguards
    startPump = max(1, floor(totalRowsPump * pump_start_frac) + 1);
    endPump = min(totalRowsPump, floor(totalRowsPump * pump_end_frac));

    startLeak = max(1, floor(totalRowsLeak * leak_start_frac) + 1);
    endLeak = min(totalRowsLeak, floor(totalRowsLeak * leak_end_frac));

    % Slice the data
    data_pumping = data_pumping(startPump:endPump, :);
    data_leaking = data_leaking(startLeak:endLeak, :);

    % Determine atmospheric pressure
    if height(data_initial) >= 1
        P_atm = median(data_initial.pressure); % Median atmospheric pressure
    else
        P_atm = 101325; % Default to standard atmospheric pressure (Pa)
    end

    % Guard for minimum rows required for SG filter
    desired_window_size = 11;      % Desired window size (must be odd)
    poly_order = 3;                % Polynomial order

    % Function to calculate appropriate window size
    calculate_window_size = @(n) min(desired_window_size, n); % Ensure window size does not exceed data length
    adjust_to_odd = @(w) w + mod(w-1, 2); % Ensure window size is odd

    % Pumping Phase
    num_pumping = height(data_pumping);
    window_size_pump = calculate_window_size(num_pumping);
    if window_size_pump < poly_order + 2
        error('Not enough data points in pumping phase to apply the Savitzky-Golay filter with the given polynomial order.');
    end
    window_size_pump = adjust_to_odd(window_size_pump);

    % Leaking Phase
    num_leaking = height(data_leaking);
    window_size_leak = calculate_window_size(num_leaking);
    if window_size_leak < poly_order + 2
        error('Not enough data points in leaking phase to apply the Savitzky-Golay filter with the given polynomial order.');
    end
    window_size_leak = adjust_to_odd(window_size_leak);

    % ================================
    % 1. Apply SG Filter to Pumping Data
    % ================================

    % Extract pumping pressure and time
    pumping_pressure = data_pumping.pressure;
    pumping_time = data_pumping.time;

    % Compute sampling interval (assuming uniform sampling)
    dt_pumping = mean(diff(pumping_time));
    if any(abs(diff(pumping_time) - dt_pumping) > 1e-6)
        warning('Non-uniform sampling detected in pumping data. Results may be inaccurate.');
    end

    % Compute the Savitzky-Golay filter coefficients
    try
        [sg_coeffs_pump, g_pump] = sgolay(poly_order, window_size_pump);

        % Apply the filter to compute the smoothed pressure
        smoothed_pumping_pressure = conv(pumping_pressure, sg_coeffs_pump(:,1), 'same');

        % Compute the first derivative
        pressure_rate_pumping = conv(pumping_pressure, -g_pump(:,2), 'same') / dt_pumping;
    catch ME
        error('Error applying Savitzky-Golay filter to pumping data: %s', ME.message);
    end

    % ================================
    % 2. Apply SG Filter to Leaking Data
    % ================================

    % Extract leaking pressure and time
    leaking_pressure = data_leaking.pressure;
    leaking_time = data_leaking.time;

    % Compute sampling interval (assuming uniform sampling)
    dt_leaking = mean(diff(leaking_time));
    if any(abs(diff(leaking_time) - dt_leaking) > 1e-6)
        warning('Non-uniform sampling detected in leaking data. Results may be inaccurate.');
    end

    % Compute the Savitzky-Golay filter coefficients
    try
        [sg_coeffs_leak, g_leak] = sgolay(poly_order, window_size_leak);

        % Apply the filter to compute the smoothed pressure
        smoothed_leaking_pressure = conv(leaking_pressure, sg_coeffs_leak(:,1), 'same');

        % Compute the first derivative
        pressure_rate_leaking = conv(leaking_pressure, -g_leak(:,2), 'same') / dt_leaking;
    catch ME
        error('Error applying Savitzky-Golay filter to leaking data: %s', ME.message);
    end

    % ================================
    % 3. Populate Qpumping and Qleaking
    % ================================

    % Remove NaN values that may result from filtering at the edges
    valid_indices_pumping = ~isnan(pressure_rate_pumping);
    valid_indices_leaking = ~isnan(pressure_rate_leaking);

    % Populate Qpumping: [Pressure Rate (Pa/s), Operating Pressure (Pa)]
    Qpumping = [pressure_rate_pumping(valid_indices_pumping), smoothed_pumping_pressure(valid_indices_pumping)];

    % Populate Qleaking: [Pressure Rate (Pa/s), Operating Pressure (Pa)]
    Qleaking = [pressure_rate_leaking(valid_indices_leaking), smoothed_leaking_pressure(valid_indices_leaking)];

    % ================================
    % 4. Compute Logarithmic Pressure Leak Rate (Optional)
    % ================================

    % Compute pressure differences
    leak_diff = leaking_pressure(2:end) - leaking_pressure(1:end-1);

    % Ensure positive differences before taking log
    valid_log_indices = leak_diff > 0;

    % Extract the corresponding subset of leaking_time
    leaking_time_subset = leaking_time(2:end);
    leaking_time_valid = leaking_time_subset(valid_log_indices);

    % Populate ln_dP_leak: [ln(dP), Time]
    if any(valid_log_indices)
        ln_dP_leak = [log(leak_diff(valid_log_indices)), leaking_time_valid];
    else
        ln_dP_leak = [];
        warning('No positive pressure differences found for logarithmic leak rate calculation.');
    end

    % ================================
    % 5. Plotting (Optional)
    % ================================

    figure;

    % Subplot 1: Pumping Rate vs Operating Pressure
    subplot(2, 1, 1); % 2 rows, 1 column, 1st plot
    plot(Qpumping(:, 2), Qpumping(:, 1), '-b', 'LineWidth', 1.5);
    xlabel('Operating Pressure (Pa)');
    ylabel('Pumping Rate (Pa/s)');
    title('Pumping Rate vs Operating Pressure');
    grid on;
    hold on;
    % Optionally, plot a fitted curve or markers
    hold off;

    % Subplot 2: Leaking Rate vs Operating Pressure
    subplot(2, 1, 2); % 2 rows, 1 column, 2nd plot
    plot(Qleaking(:, 2), Qleaking(:, 1), '-r', 'LineWidth', 1.5);
    xlabel('Operating Pressure (Pa)');
    ylabel('Leaking Rate (Pa/s)');
    title('Leaking Rate vs Operating Pressure');
    grid on;
    hold on