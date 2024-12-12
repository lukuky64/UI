function [dQ_dP] = deriveRate(Q_P)

    % Expected input is a 2D array
    % pressure rate in first column, pressure in second column

    dQ_dP = zeros((height(Q_P) - 1), 2); 

    for i=1:((height(Q_P)) - 1)
        dQ_dP(i, 1) = round(((Q_P(i + 1, 1) - Q_P(i, 1))/(Q_P(i + 1, 2) - Q_P(i, 2))), 4);

        dQ_dP(i, 2) = Q_P(i, 2); % operating point
    end
end

