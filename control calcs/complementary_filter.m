function [filtered_data] = complementary_filter(data, alpha)
    % Complementary filter for smoothing the first column of data
    % Input:
    %   - data: NxM matrix (N rows, M columns)
    %   - alpha: Smoothing factor (0 < alpha <= 1)
    % Output:
    %   - filtered_data: NxM matrix with the first column filtered

    % Validate alpha
    if (alpha < 0 || alpha >= 1)
        error('Alpha must be between 0 and 1');
    end

    % Validate input data
    if isempty(data) || size(data, 2) < 1
        error('Input data must have at least one column.');
    end

    % Preallocate filtered data with the same size as input
    filtered_data = zeros(size(data));

    % Initialize the first row of filtered data
    filtered_data(1, :) = data(1, :);

    % Apply complementary filter
    for i = 2:height(data)
        % Filter the first column
        filtered_data(i, 1) = (1 - alpha) * data(i, 1) + alpha * filtered_data(i-1, 1);
        
        % Copy other columns directly
        filtered_data(i, 2:end) = data(i, 2:end);
    end
end