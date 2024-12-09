function [data] = bin_data(data_leaking, data_pumping, numBins)
    % Expected input data format for each input: 
    % column 1 - (Pressure Change array)
    % column 2 - (Operating pressure array)

    % Find largest common pressure of both inputs
    min_leaking_pressure = min(data_leaking(:, 2));
    min_pumping_pressure = min(data_pumping(:, 2));

    max_leaking_pressure = max(data_leaking(:, 2));
    max_pumping_pressure = max(data_pumping(:, 2));

    % Select values that are within the range of both datasets
    common_min = ceil(max(min_leaking_pressure, min_pumping_pressure));
    common_max = floor(min(max_leaking_pressure, max_pumping_pressure));

    range = common_max - common_min;
    bin_width = range / numBins;

    % Preallocate data to store results for each bin
    data = zeros(numBins, 3); % [mean_binned_leaking, mean_binned_pumping, operating_pressure]
    
    % Break range into bins
    for i = 1:numBins
        lower_bounds = common_min + bin_width * (i - 1);
        upper_bounds = common_min + bin_width * i;

        % Filter data within the current bin
        binned_leaking = data_leaking(data_leaking(:, 2) >= lower_bounds & data_leaking(:, 2) <= upper_bounds, :);
        binned_pumping = data_pumping(data_pumping(:, 2) >= lower_bounds & data_pumping(:, 2) <= upper_bounds, :);

        % Compute mean values for the bin
        mean_binned_leaking = mean(binned_leaking(:, 1), 'omitnan'); % Omit NaN to avoid errors
        mean_binned_pumping = mean(binned_pumping(:, 1), 'omitnan'); % Omit NaN to avoid errors
        operating_pressure = mean([binned_leaking(:, 2); binned_pumping(:, 2)], 'omitnan'); % Combined mean pressure

        % Store results in the data matrix
        data(i, :) = [mean_binned_leaking, mean_binned_pumping, operating_pressure];
    end
end