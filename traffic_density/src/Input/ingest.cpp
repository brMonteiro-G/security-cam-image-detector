
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <utility>
#include <string>

// Returns: pair<avenue_name, generated_image_path>
std::pair<std::string, std::string> ingest_camera() {
    const std::string url = "https://cameras.santoandre.sp.gov.br/coi04/ID_655";
    const int camera_id = 655;
    const std::string output_dir = "../../resources/images/avenida_dos_estados";
    const int interval_seconds = 20;
    const std::string avenue_name = "Avenida dos Estados";

    std::filesystem::create_directories(output_dir);
    cv::VideoCapture cap(url);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open video stream." << std::endl;
        return {avenue_name, ""};
    }

    auto last_saved = std::chrono::steady_clock::now();
    cv::Mat frame;
    std::string last_filename;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Unable to retrieve frame." << std::endl;
            break;
        }

        // Save a screenshot every INTERVAL_SECONDS
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_saved).count();

        if (elapsed_seconds >= interval_seconds) {
            last_filename = output_dir + "/screenshot_" + std::to_string(camera_id) + "_" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(current_time.time_since_epoch()).count()) + ".jpg";
            cv::imwrite(last_filename, frame);
            std::cout << "Saved: " << last_filename << std::endl;
            last_saved = current_time;
            break; // Only take one screenshot per call
        }

        // Exit if 'q' is pressed
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return {avenue_name, last_filename};
}
