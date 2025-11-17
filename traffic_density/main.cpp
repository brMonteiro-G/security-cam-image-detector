#include <iostream>
#include <string>
#include <algorithm>

// ----------------------------------------------------
// DEMO version of image_capture
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_demo() {
    std::cout << "[DEMO] Simulating image capture...\n";
    // Fake demo image data generation here
}

// ----------------------------------------------------
// LIVE version of image_capture
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_live() {
    std::cout << "[LIVE] Capturing real image from camera...\n";
    // Actual camera capture code here
    return ingest_camera()
}

// ----------------------------------------------------
// Forward declaration (to be implemented in traffic_density.cpp)
void analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

// ----------------------------------------------------
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
            auto [report] = analyzeTrafficDensity();
            sendTrafficNotification(
                avenueName,
                report,
            );

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    else if (mode == "live") {
        while (true) {
            auto [avenueName, imagePath] = image_capture_live();
            auto [report] = analyzeTrafficDensity();
            sendTrafficNotification(
                avenueName,
                report,
            );

            std::this_thread::sleep_for(std::chrono::seconds(36));
        }
    }
    else {
        std::cout << "Invalid mode.\n";
        return 1;
    }

    return 0;
}

