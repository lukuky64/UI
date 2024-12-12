function [k_leak] = get_leak_constant(ln_dP_leak)
    % Function to calculate the leak rate constant k_leak from ln(Delta P) vs. time data
    %
    % Input:
    %   ln_dP_leak - N x 2 matrix where:
    %       - Column 1: ln(Delta P) values
    %       - Column 2: Corresponding time values
    %
    % Output:
    %   k_leak - Calculated leak rate constant

     % Remove rows from ln_dP_leak that contain Inf or NaN values. This can happen
     % when dP is zero or negative.
    ln_dP_leak = ln_dP_leak(~any(isinf(ln_dP_leak) | isnan(ln_dP_leak), 2), :);

    % Extract ln(Delta P) and time arrays from the input matrix
    ln_delta_P_array = ln_dP_leak(:, 1);
    time_array = ln_dP_leak(:, 2);

    % Ensure that the data arrays are column vectors
    ln_delta_P_array = ln_delta_P_array(:);
    time_array = time_array(:);

    % Check that the input arrays have the same length
    if length(ln_delta_P_array) ~= length(time_array)
        error('The ln(Delta P) and time arrays must have the same length.');
    end

    % Perform linear regression using polyfit
    p = polyfit(time_array, ln_delta_P_array, 1);

    m = p(1); % Slope

    % Calculate k_leak as the negative of the slope
    k_leak = -m;

    % Plot the data and the linear fit
    figure;
    plot(time_array, ln_delta_P_array, 'bo', 'MarkerFaceColor', 'b');
    hold on;
    % Calculate fitted ln(Delta P) values using the linear fit
    ln_delta_P_fit = polyval(p, time_array);
    plot(time_array, ln_delta_P_fit, 'r-', 'LineWidth', 2);
    hold off;
    grid on;
    xlabel('Time (s)');
    ylabel('ln(\Delta P)');
    title('Linear Fit to ln(\Delta P) vs. Time');
    legend('Data', 'Linear Fit', 'Location', 'Best');

    pause(0.5);
    
    close(gcf);
end