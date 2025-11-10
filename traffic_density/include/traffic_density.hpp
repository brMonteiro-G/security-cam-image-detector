#ifndef TRAFFIC_DENSITY_HPP
#define TRAFFIC_DENSITY_HPP

#include <opencv2/opencv.hpp>

class TrafficDensity {
public:
    TrafficDensity(double threshold);
    double computeDensity(const cv::Mat& frame);
    void analyzeFrame(const cv::Mat& frame);
    void drawDebugGrid(cv::Mat& frame, int step = 100);

private:
    double threshold_;
};

#endif
