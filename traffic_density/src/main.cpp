#include <opencv2/opencv.hpp>
#include "traffic_density.hpp"

int main() {
    cv::Mat frame = cv::imread("images/sample.jpg");
    if (frame.empty()) {
        std::cerr << "Error: Could not load image." << std::endl;
        return -1;
    }

    TrafficDensity detector(0.4);
    detector.analyzeFrame(frame);

    // Optional: visualize mask
    // cv::imshow("Traffic Frame", frame);
    // cv::waitKey(0);

    return 0;
}
