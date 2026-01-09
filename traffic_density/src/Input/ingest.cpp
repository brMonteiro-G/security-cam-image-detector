#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include "gui_compat.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <utility>
#include <string>
#include <cstdlib>


std::pair<std::string, std::string> ingest_camera() {
    static bool first_time = true;  // Track first call
    
    const std::string url = "https://cameras.santoandre.sp.gov.br/coi02/ID_074";
    const int camera_id = 074;
    
    // Use /tmp in Lambda (ENVIRONMENT != "local"), otherwise use local path
    const char* env = std::getenv("ENVIRONMENT");
    const bool isLocal = (env && std::string(env) == "local");
    const std::string output_dir = isLocal ? 
        "../resources/images/avenida_dos_estados" : 
        "/tmp/avenida_dos_estados";
    
    const int interval_seconds = 20;
    const std::string avenue_name = "Avenida dos Estados";

    std::filesystem::create_directories(output_dir);

    // Quick network reachability check to avoid blocking VideoCapture on unreachable URLs
    // Use curl to perform a HEAD request with a short timeout. If curl is not available
    // in an environment, this will return non-zero and we will skip opening the stream.
    {
        std::string headCmd = "curl -sS --max-time 5 -I '" + url + "' > /dev/null 2>&1";
        int rc = std::system(headCmd.c_str());
        if (rc != 0) {
            std::cerr << "Error: Unable to reach camera URL (curl check failed)\n";
            return {avenue_name, ""};
        }
    }

        std::cout << "[INGEST] Attempting to open camera stream: " << url << "\n";
        cv::VideoCapture cap;
        // Try to open the stream with a short retry loop so we fail fast in Lambda
        const int max_open_attempts = 5;
        int attempt = 0;
        while (attempt < max_open_attempts) {
            if (cap.open(url)) break;
            attempt++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (!cap.isOpened()) {
            std::cerr << "[INGEST ERROR] Unable to open stream after " << max_open_attempts << " attempts\n";
            return {avenue_name, ""};
        }

    cv::Mat frame;

    // ---------- FIRST CALL ONLY ----------
    if (first_time) {
        std::cout << "Preview mode (first call only).\n";
        std::cout << "Press SPACE to capture the current viewpoint.\n";
        std::cout << "Press Q to quit without capturing.\n";

        // Only show GUI in local environment
        const char* env = std::getenv("ENVIRONMENT");
        bool isLocal = (env && std::string(env) == "local");
        
        if (isLocal) {
            cv::namedWindow("Camera Preview", cv::WINDOW_AUTOSIZE);
        }
        
        while (true) {
                // Read with timeout/retries to avoid blocking indefinitely
                bool gotFrame = false;
                const int max_frame_attempts = 5;
                for (int i = 0; i < max_frame_attempts; ++i) {
                    cap >> frame;
                    if (!frame.empty()) { gotFrame = true; break; }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                if (!gotFrame) {
                    std::cerr << "[INGEST ERROR] Empty frame after retries\n";
                    break;
                }

            if (isLocal) {
                cv::Mat displayFrame;
                cv::resize(frame, displayFrame, cv::Size(1280, 720));
                cv::imshow("Camera Preview", displayFrame);
                int key = cv::waitKey(1);

                if (key == ' ') {  // SPACE pressed
                    break;
                }
                if (key == 'q' || key == 'Q') {
                    cap.release();
                    cv::destroyWindow("Camera Preview");
                    return {avenue_name, ""};
                }
            } else {
                // In Lambda/cloud, just capture the frame immediately
                break;
            }
        }

        if (isLocal) {
            cv::destroyWindow("Camera Preview");
        }
        first_time = false;  // <--- future calls will skip preview
    }
    // ---------- SUBSEQUENT CALLS ----------
    else {
        cap >> frame;  // just grab one frame immediately
        if (frame.empty()) {
            std::cerr << "Error: Empty frame\n";
            return {avenue_name, ""};
        }
    }

    // Save the captured frame
    long ts = std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();

    std::string filename =
        output_dir + "/screenshot_" + std::to_string(camera_id) + "_" +
        std::to_string(ts) + ".jpg";

    cv::imwrite(filename, frame);
    cap.release();

    return {avenue_name, filename};
}
