#include <string>

// Forward declaration (to be implemented in traffic_density.cpp)
std::string analyzeTrafficDensity(const std::string& imagePath, const std::string& avenueName);

// Image capture
std::pair<std::string, std::string> ingest_camera();

int main() {
	auto [avenueName, imagePath] = ingest_camera();
    printf("Avenue: %s\nImage Path: %s\n", avenueName.c_str(), imagePath.c_str());
    
    std::string report = analyzeTrafficDensity(imagePath, avenueName);

    // sendTrafficNotification(
    // avenueName,
    // report,
    // );

	return 0;
}
