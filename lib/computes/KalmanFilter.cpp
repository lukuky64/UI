#include "KalmanFilter.hpp"

KalmanFilter::KalmanFilter(float processNoise, float measurementNoise, float estimatedError, float initialValue)
{
    Q = processNoise;
    R = measurementNoise;
    P = estimatedError;
    X = initialValue; // Initial estimate
}

void KalmanFilter::update(float measurement)
{
    // Prediction update
    P = P + Q;

    // Measurement update
    K = P / (P + R);
    X = X + K * (measurement - X);
    P = (1 - K) * P;
}

float KalmanFilter::getValue()
{
    return X;
}
