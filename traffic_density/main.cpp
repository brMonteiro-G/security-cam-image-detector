#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>


#include <utility>

// Image capture
std::pair<std::string, std::string> ingest_camera();
std::string test_static_image(const std::string& imagePath, const std::string& avenueName);

// ----------------------------------------------------
// DEMO version of image_capture
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_demo() {
    std::cout << "[DEMO] Simulating image capture...\n";

    // Generator-like function that returns an image from the folder every
    // time it is called. When done it loops to the beginning, keeping the demo
    // running with only a few images

    static std::vector<std::filesystem::path> files;
    static size_t index = 0;

    // Load files only once
    if (files.empty()) {
        std::string folder = "./resources/images/demo";
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            if (entry.is_regular_file())
                files.push_back(entry.path());
        }
    }

    if (files.empty()) {
        return { "NoFiles", "" };
    }

    // Get the current file
    const auto& path = files[index];

    // Move to next, wrap around
    index = (index + 1) % files.size();

    // Generate static name (ex: use filename without extension)
    std::string avenueName = "Avenida dos Estados";

    return { avenueName, path.string() };

}

// ----------------------------------------------------
// LIVE version of image_capture
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_live() {
    std::cout << "[LIVE] Capturing real image from camera...\n";
    // Actual camera capture code here
    return ingest_camera();
}

// ----------------------------------------------------
// Flow Control helper function
// ----------------------------------------------------
bool userRequestedExit() {
    return std::cin.rdbuf()->in_avail() > 0;
}


// ----------------------------------------------------
// Forward declaration (to be implemented in traffic_density.cpp)
std::string analyzeTrafficDensity(const std::string& imagePath);

// ----------------------------------------------------

// Simple notification function that logs to cout
void sendTrafficNotification(const std::string& avenueName, const std::string& report) {
    std::cout << "[NOTIFICATION] Traffic report for " << avenueName << ": " << report << "\n";
}



int main(int argc, char* argv[]) {

    std::string mode;
    if (argc > 1) mode = argv[1];
    else {
        std::cout << "Select mode (demo/live): ";
        std::cin >> mode;
    }

    // Convert to lowercase
    for (auto &c : mode) c = std::tolower(c);

    if (mode == "demo") {
        while (true) {
            auto [avenueName, imagePath] = image_capture_demo();

            if (imagePath.empty()) {
                std::cout << "No demo images. Exiting demo mode.\n";
                break;
            }

            std::string processedImagePath = test_static_image(imagePath, avenueName);
            std::string report = analyzeTrafficDensity(processedImagePath);

            sendTrafficNotification(
                avenueName,
                report
            );

            // Wait 5 seconds, exit early if user presses ENTER
            for (int i = 0; i < 50; ++i) {
                if (userRequestedExit()) {
                    std::string line;
                    std::getline(std::cin, line); // consume input
                    std::cout << "Exiting demo mode...\n";
                    return 0; // or break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    else if (mode == "live") {
        while (true) {
            auto [avenueName, imagePath] = image_capture_live();
            printf("Captured image for %s at %s\n", avenueName.c_str(), imagePath.c_str());
            std::string processedImagePath = test_static_image(imagePath, avenueName);
            
            std::string report = analyzeTrafficDensity(processedImagePath);
            sendTrafficNotification(
                avenueName,
                report
            );

            // Wait 36 seconds for camera to get back into position, exit early if user presses ENTER
            for (int i = 0; i < 360; ++i) {
                if (userRequestedExit()) {
                    std::string line;
                    std::getline(std::cin, line); // consume input
                    std::cout << "Exiting demo mode...\n";
                    return 0; // or break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    else {
        std::cout << "Invalid mode.\n";
        return 1;
    }

    return 0;
}

