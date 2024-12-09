#ifndef KALMANFILTER_HPP
#define KALMANFILTER_HPP

class KalmanFilter
{
public:
    KalmanFilter(float processNoise, float measurementNoise, float estimatedError, float initialValue);
    void update(float measurement);
    float getValue();

private:
    float Q; // Process noise covariance
    float R; // Measurement noise covariance
    float P; // Estimate error covariance
    float K; // Kalman gain
    float X; // Value
};

#endif // KALMANFILTER_HPP