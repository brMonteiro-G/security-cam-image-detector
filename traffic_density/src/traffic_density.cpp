#include "traffic_density.hpp"
#include <iostream>
#include <chrono>
#include <thread>


TrafficDensity::TrafficDensity(double threshold) : threshold_(threshold) {}

using namespace std::chrono_literals;


double TrafficDensity::computeDensity(const cv::Mat& frame) {
    cv::Mat hsv, mask;

    // Convert to HSV color space
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Define range for vehicle-like colors (adjust as needed)
    cv::Scalar low(0, 0, 100);
    cv::Scalar high(180, 80, 255);

    // Threshold the image
    cv::inRange(hsv, low, high, mask);

    // Find contours of detected areas
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


    cv::Mat maskROI = cv::Mat::zeros(frame.size(), CV_8UC1);
    cv::Mat hsv, mask, debugFrame;


    // // Define polygon points that roughly outline the road area
    // std::vector<cv::Point> roadPts = {
    //     cv::Point(150, 400),
    //     cv::Point(1100, 400),
    //     cv::Point(1280, 720),
    //     cv::Point(0, 720)
    // };

    // // Fill the polygon white in the mask
    // cv::fillPoly(maskROI, std::vector<std::vector<cv::Point>>{roadPts}, cv::Scalar(255));

    // // Apply it to your threshold mask before finding contours
    // cv::bitwise_and(mask, maskROI, mask);


    frame.copyTo(debugFrame);

    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::Scalar low(0, 0, 100);
    cv::Scalar high(180, 80, 255);
    cv::inRange(hsv, low, high, mask);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    double vehicleArea = 0.0;
    for (const auto& c : contours) {
        vehicleArea += cv::contourArea(c);

        // Draw rounded bounding boxes for visualization
        cv::Rect rect = cv::boundingRect(c);
        cv::rectangle(debugFrame, rect, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    double frameArea = frame.cols * frame.rows;
    double density = vehicleArea / frameArea;

    // --- 🪟 Display results ---
    //cv::imshow("Original Frame", frame);
    //cv::imshow("Threshold Mask", mask);
    drawDebugGrid(frame);

    cv::imshow("Detected Vehicles", debugFrame);

    std::cout << "Estimated density: " << density << std::endl;
    if (density > threshold_) {
        cv::waitKey(1);  // refresh windows
        std::this_thread::sleep_for(5000ms);

        std::cout << "⚠️  Heavy traffic detected!" << std::endl;

    } else {
        cv::waitKey(1);  // refresh windows
        std::this_thread::sleep_for(5000ms);

        std::cout << "✅  Light traffic." << std::endl;

    }

    
}
