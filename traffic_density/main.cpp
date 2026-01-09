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
#ifdef USE_AWS_SNS
#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#endif

// Load .env file when running in local development (ENVIRONMENT=local).
// Lines formatted as KEY=VALUE. Existing environment variables are not overwritten.
void loadEnvFile(const std::string& path) {
    const char* env = std::getenv("ENVIRONMENT");
    if (env && std::string(env) != "local") {
        std::cout << "[ENV] Not local environment, skipping .env load\n";
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[ENV] .env file not found: " << path << "\n";
        return;
    }

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        auto l = line.find_first_not_of(" \t\r\n");
        if (l == std::string::npos) continue;
        auto r = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(l, r - l + 1);
        if (trimmed.empty() || trimmed[0] == '#') continue;
        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trimmed.substr(0, eq);
        std::string value = trimmed.substr(eq + 1);

        // Remove surrounding quotes from value if present
        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        // Skip if already set in environment
        if (std::getenv(key.c_str())) {
            std::cout << "[ENV] Skip " << key << " (already set in environment)\n";
            continue;
        }

        setenv(key.c_str(), value.c_str(), 0);
        ++count;
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
// DEMO version - uses pre-saved images
// ----------------------------------------------------
std::pair<std::string, std::string> image_capture_demo() {
    static int imageIndex = 0;
    static std::vector<std::string> demoImages;
    
    // Initialize demo images list on first call
    if (demoImages.empty()) {
        // Check if we're in Lambda or local environment
        const char* env = std::getenv("ENVIRONMENT");
        bool isLocal = (env && std::string(env) == "local");
        
        std::string demoDir = isLocal ? 
            "./resources/images/demo" : 
            "/opt/resources/images/demo";
        
        std::cout << "[DEMO] Looking for demo images in: " << demoDir << "\n";
        
        // List all .png and .jpg files in demo directory
        if (std::filesystem::exists(demoDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(demoDir)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        demoImages.push_back(entry.path().string());
                        std::cout << "[DEMO] Found image: " << entry.path().filename() << "\n";
                    }
                }
            }
        } else {
            std::cerr << "[DEMO ERROR] Demo directory not found: " << demoDir << "\n";
        }
        
        if (demoImages.empty()) {
            std::cerr << "[DEMO ERROR] No demo images found\n";
            return {"Avenida dos Estados", ""};
        }
        
        std::cout << "[DEMO] Loaded " << demoImages.size() << " demo images\n";
    }
    
    // Return empty if no images available
    if (demoImages.empty()) {
        return {"Avenida dos Estados", ""};
    }
    
    // Cycle through demo images
    std::string imagePath = demoImages[imageIndex];
    imageIndex = (imageIndex + 1) % demoImages.size();
    
    std::cout << "[DEMO] Using image: " << imagePath << "\n";
    return {"Avenida dos Estados", imagePath};
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
        // Create SNS client with custom configuration for VPC
        Aws::Client::ClientConfiguration clientConfig;
        clientConfig.region = "us-east-1";
        clientConfig.connectTimeoutMs = 10000;  // 10 seconds
        clientConfig.requestTimeoutMs = 30000;  // 30 seconds
        
        Aws::SNS::SNSClient snsClient(clientConfig);
        
        std::cout << "[SNS] Client created with extended timeout\n";
        
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

    // Debug: print key environment variables so CloudWatch shows them on start
    const char* env_run = std::getenv("ENVIRONMENT");
    const char* cam_url = std::getenv("CAMERA_STREAM_URL");
    const char* sns_arn = std::getenv("AWS_SNS_TOPIC_ARN");
    const char* s3_bucket = std::getenv("S3_BUCKET_NAME");
    std::cout << "[STARTUP] ENVIRONMENT=" << (env_run ? env_run : "(unset)")
              << " CAMERA_STREAM_URL=" << (cam_url ? cam_url : "(unset)")
              << " AWS_SNS_TOPIC_ARN=" << (sns_arn ? sns_arn : "(unset)")
              << " S3_BUCKET_NAME=" << (s3_bucket ? s3_bucket : "(unset)") << "\n";

    std::string mode;
    if (argc > 1) mode = argv[1];
    else {
        std::cout << "Select mode (demo/live): ";
        std::cin >> mode;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Convert to lowercase
    for (auto &c : mode) c = std::tolower(c);

    // Determine if running locally or in Lambda
    bool isLocalRun = (env_run && std::string(env_run) == "local");

    if (mode == "demo") {
        // Check if running in Lambda (non-local environment)
        const char* env = std::getenv("ENVIRONMENT");
        bool isLocal = (env && std::string(env) == "local");
        
        if (isLocal) {
            std::cout << "Entering demo mode. Press ENTER at any time to stop.\n";
            while (true) {
                auto [avenueName, imagePath] = image_capture_demo();

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
        } else {
            // In Lambda: process just one image and exit
            std::cout << "Entering demo mode (Lambda: single execution).\n";
            auto [avenueName, imagePath] = image_capture_demo();

            if (imagePath.empty()) {
                std::cout << "No demo images available.\n";
                return 1;
            }

            std::string processedImagePath = test_static_image(imagePath, avenueName);
            std::string report = analyzeTrafficDensity(processedImagePath, avenueName);

            sendTrafficNotification(
                avenueName,
                report
            );
            
            std::cout << "Demo mode completed successfully.\n";
        }
    }

    else if (mode == "live") {
        // If running locally, behave interactively. In Lambda/non-local, run a single
        // capture/process/report cycle and exit to avoid long-running functions.
        if (isLocalRun) {
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

                std::cout << "[FLOW] live: calling image_capture_live()\n";
                auto [avenueName, imagePath] = image_capture_live();
                std::cout << "[FLOW] live: image_capture_live returned path='" << imagePath << "'\n";
                if (imagePath.empty()) {
                    std::cout << "[FLOW] live: no image returned, sleeping 1s\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }

                auto now = clock::now();
                bool shouldReport = (now - lastReport) >= std::chrono::seconds(30);

                std::string analysisPath = imagePath;
                if (shouldReport) {
                    std::cout << "[FLOW] live: preprocessing image: " << imagePath << "\n";
                    std::string processedImagePath = test_static_image(imagePath, avenueName);
                    std::cout << "[FLOW] live: preprocessing returned '" << processedImagePath << "'\n";
                    if (!processedImagePath.empty()) {
                        analysisPath = processedImagePath;
                    }
                }

                std::cout << "[FLOW] live: starting analysis on: " << analysisPath << "\n";
                std::string report = analyzeTrafficDensity(analysisPath, avenueName);
                std::cout << "[FLOW] live: analysis report='" << report << "'\n";

                if (shouldReport && !report.empty()) {
                    std::cout << "[FLOW] live: sending notification\n";
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
        } else {
            // Lambda: single-run live processing (fetch one frame and report)
            std::cout << "Entering live mode (Lambda: single execution).\n";

            std::cout << "[FLOW] live(Lambda): calling image_capture_live()\n";
            auto [avenueName, imagePath] = image_capture_live();
            std::cout << "[FLOW] live(Lambda): image_capture_live returned path='" << imagePath << "'\n";

            if (imagePath.empty()) {
                std::cout << "No image captured from live stream.\n";
                return 1;
            }

            std::cout << "[FLOW] live(Lambda): preprocessing image: " << imagePath << "\n";
            std::string processedImagePath = test_static_image(imagePath, avenueName);
            std::cout << "[FLOW] live(Lambda): preprocessing returned '" << processedImagePath << "'\n";

            std::cout << "[FLOW] live(Lambda): starting analysis on: " << processedImagePath << "\n";
            std::string report = analyzeTrafficDensity(processedImagePath, avenueName);
            std::cout << "[FLOW] live(Lambda): analysis report='" << report << "'\n";

            if (!report.empty()) {
                std::cout << "[FLOW] live(Lambda): sending notification\n";
                sendTrafficNotification(avenueName, report);
            }

            std::cout << "Live single-run completed.\n";
        }
    }
    else {
        std::cout << "Invalid mode.\n";
        return 1;
    }

    return 0;
}
