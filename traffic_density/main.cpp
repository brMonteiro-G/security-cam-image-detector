#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <system_error>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

// AWS SDK includes
#ifdef USE_AWS_SNS
#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#endif

// ============================================
// Load Environment Variables from .env file
// ============================================
void loadEnvFile(const std::string& filepath = "../.env") {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "[INFO] .env file not found at: " << filepath << "\n";
        std::cout << "[INFO] Using system environment variables\n";
        return;
    }

    std::cout << "[ENV] Loading environment variables from: " << filepath << "\n";
    std::string line;
    int count = 0;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the '=' separator
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        // Extract key and value
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Trim whitespace from key
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);

        // Trim whitespace and quotes from value
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        // Remove surrounding quotes if present
        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.length() - 2);
        }

        // Set environment variable (only if not already set)
        if (!std::getenv(key.c_str())) {
            setenv(key.c_str(), value.c_str(), 1);
            std::cout << "[ENV] Set " << key << "=" << (key.find("SECRET") != std::string::npos || key.find("KEY") != std::string::npos ? "***" : value) << "\n";
            count++;
        } else {
            std::cout << "[ENV] Skip " << key << " (already set in environment)\n";
        }
    }

    file.close();
    std::cout << "[ENV] Loaded " << count << " environment variables\n";
}

// Forward declaration (to be implemented in traffic_density.cpp)
std::string analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

std::string test_static_image(const std::string& imagePath, const std::string& avenueName);


// Image capture
std::pair<std::string, std::string> ingest_camera();

// int main() {
// 	auto [avenueName, imagePath] = ingest_camera();
//     printf("Avenue: %s\nImage Path: %s\n", avenueName.c_str(), imagePath.c_str());
    
//     std::string processedImagePath = test_static_image(imagePath, avenueName);

//     printf("Processed Image Path: %s\n", processedImagePath.c_str());
    
//     std::string report = analyzeTrafficDensity(processedImagePath, avenueName);

//     // sendTrafficNotification(
//     // avenueName,
//     // report,
//     // );

// }

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
std::string analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

// ----------------------------------------------------

// Helper function to sanitize string for AWS MessageDeduplicationId
// AWS requires: alphanumeric and punctuation characters only (1-128 length)
// Allowed: a-z, A-Z, 0-9, and punctuation: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
std::string sanitizeForDeduplicationId(const std::string& input) {
    std::string sanitized;
    sanitized.reserve(input.length());
    
    for (char c : input) {
        // Replace spaces with hyphens
        if (c == ' ') {
            sanitized += '-';
        }
        // Keep alphanumeric and common punctuation
        else if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == ':') {
            sanitized += c;
        }
        // Skip other characters
    }
    
    // Ensure length is between 1-128
    if (sanitized.empty()) {
        sanitized = "default";
    }
    if (sanitized.length() > 128) {
        sanitized = sanitized.substr(0, 128);
    }
    
    return sanitized;
}

// Traffic notification function with AWS SNS support
void sendTrafficNotification(const std::string& avenueName, const std::string& report) {
    // Parse the report to extract vehicle count and condition
    // Expected format: "N vehicles detected with density D. Condition: CONDITION"
    int vehicleCount = 0;
    double density = 0.0;
    std::string condition = "Unknown";
    
    // Extract vehicle count
    size_t vehiclesPos = report.find("vehicles detected");
    if (vehiclesPos != std::string::npos) {
        std::string countStr = report.substr(0, vehiclesPos);
        vehicleCount = std::stoi(countStr);
    }
    
    // Extract density
    size_t densityPos = report.find("density ");
    if (densityPos != std::string::npos) {
        size_t endPos = report.find(".", densityPos);
        if (endPos != std::string::npos) {
            std::string densityStr = report.substr(densityPos + 8, endPos - (densityPos + 8));
            density = std::stod(densityStr);
        }
    }
    
    // Extract condition
    size_t conditionPos = report.find("Condition: ");
    if (conditionPos != std::string::npos) {
        condition = report.substr(conditionPos + 11);
    }
    
    // Create JSON structure
    nlohmann::json jsonReport;
    jsonReport["avenue_name"] = avenueName;
    jsonReport["vehicles_detected"] = vehicleCount;
    jsonReport["density"] = density;
    jsonReport["condition_traffic"] = condition;
    jsonReport["timestamp"] = std::time(nullptr);
    jsonReport["timestamp_iso"] = []() {
        auto now = std::time(nullptr);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
        return std::string(buf);
    }();
    
    std::string jsonMessage = jsonReport.dump(2);  // Pretty print with 2-space indent
    
    // Always log to console
    std::cout << "[NOTIFICATION] Traffic report for " << avenueName << ":\n" << jsonMessage << "\n";
    
#ifdef USE_AWS_SNS
    // Get SNS Topic ARN from environment variable
    const char* topicArn = std::getenv("AWS_SNS_TOPIC_ARN");
    if (!topicArn) {
        std::cerr << "[WARNING] AWS_SNS_TOPIC_ARN not set. Skipping SNS notification.\n";
        return;
    }

    try {
        // Create SNS client
        Aws::SNS::SNSClient snsClient;
        
        // Prepare message
        std::string subject = "Traffic Alert: " + avenueName;
        
        // Create publish request
        Aws::SNS::Model::PublishRequest request;
        request.SetTopicArn(topicArn);
        request.SetSubject(subject);
        request.SetMessage(jsonMessage);  // Send JSON as message body
        
        // Check if this is a FIFO topic (ends with .fifo)
        std::string topicArnStr(topicArn);
        if (topicArnStr.find(".fifo") != std::string::npos) {
            // For FIFO topics, MessageGroupId is required
            request.SetMessageGroupId("traffic-alerts");
            
            // Set MessageDeduplicationId to prevent duplicates
            // Sanitize avenue name (remove spaces, keep only alphanumeric + punctuation)
            std::string sanitizedAvenue = sanitizeForDeduplicationId(avenueName);
            std::string deduplicationId = sanitizedAvenue + "-" + std::to_string(std::time(nullptr));
            request.SetMessageDeduplicationId(deduplicationId);
            
            std::cout << "[SNS] FIFO topic detected\n";
            std::cout << "[SNS] MessageGroupId: traffic-alerts\n";
            std::cout << "[SNS] MessageDeduplicationId: " << deduplicationId << "\n";
        }
        
        // Publish to SNS
        auto outcome = snsClient.Publish(request);
        
        if (outcome.IsSuccess()) {
            std::cout << "[SNS] Message published successfully. MessageId: " 
                      << outcome.GetResult().GetMessageId() << "\n";
        } else {
            std::cerr << "[SNS ERROR] Failed to publish message: " 
                      << outcome.GetError().GetMessage() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[SNS EXCEPTION] " << e.what() << "\n";
    }
#else
    std::cout << "[INFO] AWS SNS support not enabled (compile with -DUSE_AWS_SNS)\n";
#endif
}



int main(int argc, char* argv[]) {
    // Load environment variables from .env file
    loadEnvFile("../.env");

#ifdef USE_AWS_SNS
    // Initialize AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    std::cout << "[AWS] SDK initialized\n";
    
    // Ensure cleanup on exit
    std::atexit([]() {
        Aws::SDKOptions options;
        Aws::ShutdownAPI(options);
        std::cout << "[AWS] SDK shutdown\n";
    });
#endif

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
            auto [avenueName, imagePath] = image_capture_live();

            if (imagePath.empty()) {
                std::cout << "No demo images. Exiting demo mode.\n";
                break;
            }

            std::string processedImagePath = test_static_image(imagePath, avenueName);
            std::string report = analyzeTrafficDensity(processedImagePath, avenueName);

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

            std::string report = analyzeTrafficDensity(analysisPath, avenueName);

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
