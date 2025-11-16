#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

// === Get formatted timestamp ===
std::string getFormattedTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// === Send SNS Notification ===
bool sendTrafficNotification(const std::string& avenueName,
                             const std::string& report,
                             const std::string& snsTopicArn) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::SNS::SNSClient snsClient;

        std::string subject = "🚦 Heavy Traffic Alert";
        std::stringstream message;

        message << "⚠️ High traffic detected on " << avenueName << "\n\n";
        message << "📅 Date & Time: " << getFormattedTimestamp() << "\n";
        message << "📊 Report Summary:\n" << report << "\n\n";
        message << "Stay safe and consider alternative routes.";

        Aws::SNS::Model::PublishRequest request;
        request.SetTopicArn(snsTopicArn);
        request.SetSubject(subject);
        request.SetMessage(message.str());

        auto outcome = snsClient.Publish(request);

        if (outcome.IsSuccess()) {
            std::cout << "📨 SNS Notification sent successfully!" << std::endl;
            Aws::ShutdownAPI(options);
            return true;
        } else {
            std::cerr << "❌ Failed to send SNS notification: " 
                      << outcome.GetError().GetMessage() << std::endl;
            Aws::ShutdownAPI(options);
            return false;
        }
    }
    Aws::ShutdownAPI(options);
    return false;
}

// === Example Usage (can be called from main.cpp) ===
// int main() {
//     std::string avenue = "Avenida dos Estados";
//     std::string report = "Vehicle Count: 18\nTraffic Density: 0.21 (above threshold)";
//     std::string topicArn = "arn:aws:sns:us-east-1:123456789012:TrafficAlerts";
//
//     sendTrafficNotification(avenue, report, topicArn);
//     return 0;
// }
