#ifndef TRAFFIC_DENSITY_HPP
#define TRAFFIC_DENSITY_HPP

#include <opencv2/opencv.hpp>

class TrafficDensity {
public:
    TrafficDensity(double threshold);
    double computeDensity(const cv::Mat& frame);
    void analyzeFrame(const cv::Mat& frame);

private:
    double threshold_;
};

#endif
