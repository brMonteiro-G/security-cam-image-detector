#include "traffic_density.hpp"
#include <iostream>

TrafficDensity::TrafficDensity(double threshold) : threshold_(threshold) {}

double TrafficDensity::computeDensity(const cv::Mat& frame) {
    cv::Mat hsv, mask;

    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar low(0, 0, 100);
    cv::Scalar high(180, 80, 255);
    cv::inRange(hsv, low, high, mask);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    double vehicleArea = 0.0;
    for (const auto& c : contours) {
        vehicleArea += cv::contourArea(c);
    }

    double frameArea = frame.cols * frame.rows;
    return vehicleArea / frameArea;
}

void TrafficDensity::analyzeFrame(const cv::Mat& frame) {
    double density = computeDensity(frame);
    std::cout << "Estimated density: " << density << std::endl;

    if (density > threshold_) {
        std::cout << "⚠️  Heavy traffic detected!" << std::endl;
    } else {
        std::cout << "✅  Light traffic." << std::endl;
    }
}
