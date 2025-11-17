#include <string>

// Forward declaration (to be implemented in traffic_density.cpp)
void analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

int main() {
    
    // preprocess_static("../../resources/images/gemini_sec_cam_traffic.png"); 

	auto [avenueName, imagePath] = ingest_camera();
    auto [report] = analyzeTrafficDensity(imagePath, avenueName);

    sendTrafficNotification(
    avenueName,
    report,
    );

	return 0;
}
