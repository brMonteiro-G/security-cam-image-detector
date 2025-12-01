#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <system_error>

// Forward declaration (to be implemented in traffic_density.cpp)
std::string analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

std::string test_static_image(const std::string& imagePath, const std::string& avenueName);


// Image capture
std::pair<std::string, std::string> ingest_camera();

int main() {
	auto [avenueName, imagePath] = ingest_camera();
    printf("Avenue: %s\nImage Path: %s\n", avenueName.c_str(), imagePath.c_str());
    
    std::string processedImagePath = test_static_image(imagePath, avenueName);

    printf("Processed Image Path: %s\n", processedImagePath.c_str());
    
    std::string report = analyzeTrafficDensity(processedImagePath, avenueName);

    // sendTrafficNotification(
    // avenueName,
    // report,
    // );

}

// ----------------------------------------------------
// LIVE version of image_capture
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_live() {
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
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Convert to lowercase
    for (auto &c : mode) c = std::tolower(c);

    if (mode == "demo") {
        std::cout << "Entering demo mode. Press ENTER at any time to stop.\n";
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
                    std::cout << "Exit requested. Leaving demo mode...\n";
                    return 0; // or break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    else if (mode == "live") {
        std::cout << "Entering live mode. Press ENTER at any time to stop.\n";
        using clock = std::chrono::steady_clock;
        auto lastReport = clock::now() - std::chrono::seconds(30);

        while (true) {
            if (userRequestedExit()) {
                std::string line;
                std::getline(std::cin, line); // consume input
                std::cout << "Exit requested. Leaving live mode...\n";
                return 0;
            }

            auto [avenueName, imagePath] = image_capture_live();
            if (imagePath.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            auto now = clock::now();
            bool shouldReport = (now - lastReport) >= std::chrono::seconds(30);

            std::string analysisPath = imagePath;
            if (shouldReport) {
                std::string processedImagePath = test_static_image(imagePath, avenueName);
                if (!processedImagePath.empty()) {
                    analysisPath = processedImagePath;
                }
            }

            std::string report = analyzeTrafficDensity(analysisPath);

            if (shouldReport && !report.empty()) {
                sendTrafficNotification(
                    avenueName,
                    report
                );
                lastReport = now;
            }

            if (!imagePath.empty()) {
                std::error_code ec;
                std::filesystem::remove(imagePath, ec);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    else {
        std::cout << "Invalid mode.\n";
        return 1;
    }

    return 0;
}
