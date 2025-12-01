#include <string>

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

	return 0;
}
