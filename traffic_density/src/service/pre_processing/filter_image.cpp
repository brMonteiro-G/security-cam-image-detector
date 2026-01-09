#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "gui_compat.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <string>

using namespace cv;
using namespace std;

// Helper function: Apply CLAHE to HSV
Mat apply_clahe_hsv(const Mat& frame) {
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);
    
    vector<Mat> channels;
    split(hsv, channels);
    
    Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8, 8));
    clahe->apply(channels[2], channels[2]);
    
    merge(channels, hsv);
    
    Mat result;
    cvtColor(hsv, result, COLOR_HSV2BGR);
    return result;
}

// Helper function: Apply bilateral filter
Mat apply_bilateral_filter(const Mat& frame) {
    Mat result;
    bilateralFilter(frame, result, 9, 75, 75);
    return result;
}

// Update preprocess_static to accept avenue_name
std::string preprocess_static(const Mat& frame, const std::string& avenue_name) {
    Mat result = apply_clahe_hsv(frame);
    result = apply_bilateral_filter(result);
    // result = apply_roi(result); // Uncomment if ROI is needed

    long timestamp = chrono::system_clock::to_time_t(chrono::system_clock::now());
    
    // Use /tmp in Lambda (ENVIRONMENT != "local"), otherwise use local path
    const char* env = std::getenv("ENVIRONMENT");
    bool isLocal = (env && std::string(env) == "local");
    string output_dir = isLocal ? "./resources/images/" : "/tmp/";
    
    filesystem::create_directories(output_dir);

    ostringstream filename;
    filename << output_dir
             << "filtered_image_"
             << avenue_name << "_"
             << timestamp
             << ".png";
    string path = filename.str();
    imwrite(path, result);
    return path;
}

// Update test_static_image to accept avenue_name and return the processed path
std::string test_static_image(const std::string& image_path, const std::string& avenue_name) {
    if (!filesystem::exists(image_path)) {
        cout << "Image not found: " << image_path << endl;
        return "";
    }
    Mat frame = imread(image_path);
    if (frame.empty()) {
        cout << "Failed to load image: " << image_path << endl;
        return "";
    }
    std::string processed_path = preprocess_static(frame, avenue_name);

    // Only show GUI in local environment
    const char* env = std::getenv("ENVIRONMENT");
    if (env && std::string(env) == "local") {
        // Side-by-side display
        Mat processed = imread(processed_path);
        Mat combined;
        hconcat(frame, processed, combined);
        imshow("Original | Processed", combined);
    }

    return processed_path;
}