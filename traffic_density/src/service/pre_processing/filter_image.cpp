#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <filesystem>
#include <iostream>

using namespace cv;
using namespace std;

// --- UTILS ---

Mat apply_clahe_hsv(const Mat& frame) {
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    vector<Mat> channels;
    split(hsv, channels);

    Ptr<CLAHE> clahe = createCLAHE(3.0, Size(8, 8));
    clahe->apply(channels[2], channels[2]);

    merge(channels, hsv);
    Mat result;
    cvtColor(hsv, result, COLOR_HSV2BGR);
    return result;
}

Mat apply_bilateral_filter(const Mat& frame) {
    Mat result;
    bilateralFilter(frame, result, 9, 75, 75);
    return result;
}

Mat apply_roi(const Mat& frame, double start_ratio = 0.30) {
    int h = frame.rows;
    int w = frame.cols;
    int y = static_cast<int>(h * start_ratio);
    return frame(Rect(0, y, w, h - y));
}

// ======================================================
// 1️⃣ Static image pipeline
// ======================================================

// RETURNS STRING (PATH)
// RECEIVES AVENUE NAME
string preprocess_static(const Mat& frame, const string& avenue_name) {
    Mat result = apply_clahe_hsv(frame);
    result = apply_bilateral_filter(result);
    // result = apply_roi(result); // Uncomment if needed

    long timestamp = chrono::system_clock::to_time_t(chrono::system_clock::now());

    string output_dir = "./resources/images/";
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

// ======================================================
// 2️⃣ Test static image from resources
// ======================================================

string test_static_image(const string& image_path, const string& avenue_name) {
    if (!filesystem::exists(image_path)) {
        cout << "Image not found: " << image_path << endl;
        return "";
    }

    Mat frame = imread(image_path);
    if (frame.empty()) {
        cout << "Failed to load image: " << image_path << endl;
        return "";
    }

    string saved_path = preprocess_static(frame, avenue_name);

    // Side-by-side display (equivalent of np.hstack)
    Mat processed = imread(saved_path);
    Mat combined;
    hconcat(frame, processed, combined);

    imshow("Original | Processed", combined);
    waitKey(1);

    return saved_path;
}
