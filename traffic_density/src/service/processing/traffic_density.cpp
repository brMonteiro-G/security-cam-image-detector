#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <nlohmann/json.hpp>  // JSON library (https://github.com/nlohmann/json)

using namespace cv;
using namespace dnn;
using namespace std;

// === TrafficDensity Class ===
class TrafficDensity {
public:
    TrafficDensity(double threshold) : threshold_(threshold) {}

    double computeDensity(const vector<Rect>& boxes, const Mat& frame) {
        double vehicleArea = 0.0;
        for (const auto& b : boxes) {
            vehicleArea += b.area();
        }
        double frameArea = frame.cols * frame.rows;
        return vehicleArea / frameArea;
    }

    string analyzeDensity(double density) {
        if (density > threshold_) {
            return "Heavy traffic";
        } else {
            return "Light traffic";
        }
    }

private:
    double threshold_;
};

// === Rounded Box ===
void drawRoundedRectangle(Mat& img, Rect box, Scalar color, int thickness = 2) {
    int radius = static_cast<int>(min(box.width, box.height) * 0.1);
    Point tl = box.tl();
    Point br = box.br();
    int x = tl.x;
    int y = tl.y;
    int w = box.width;
    int h = box.height;

    rectangle(img, Point(x + radius, y), Point(x + w - radius, y + h), color, thickness);
    rectangle(img, Point(x, y + radius), Point(x + w, y + h - radius), color, thickness);
    circle(img, Point(x + radius, y + radius), radius, color, thickness);
    circle(img, Point(x + w - radius, y + radius), radius, color, thickness);
    circle(img, Point(x + radius, y + h - radius), radius, color, thickness);
    circle(img, Point(x + w - radius, y + h - radius), radius, color, thickness);
}

// === Timestamp (unchanged) ===
string getTimestamp() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    stringstream ss;
    ss << put_time(&local_tm, "%Y-%m-%dT%H-%M-%S");
    return ss.str();
}


namespace {
struct YoloContext {
    Net net;
    vector<String> outputLayers;
};

string analyzeTrafficDensity(const string& imagePath, const std::string& avenueName){

    printf("Starting traffic density analysis...\n");

    // Use consistent paths for model files
    string weightsPath = "../resources/models/yolov3.weights";
    string configPath = "../resources/models/yolov3.cfg";

    // Convert to absolute paths to avoid any path resolution issues
    weightsPath = std::filesystem::absolute(weightsPath).string();
    configPath = std::filesystem::absolute(configPath).string();

    printf("Absolute paths: weights=%s, config=%s\n", weightsPath.c_str(), configPath.c_str());

    if (!std::filesystem::exists(weightsPath) || !std::filesystem::exists(configPath)) {
        std::cerr << "YOLO model files not found!" << std::endl;
        std::cerr << "Looking for: " << weightsPath << " and " << configPath << std::endl;
        return "Error: YOLO model files not found";
    }

    // Load YOLO model using Darknet-specific function
    printf("Loading YOLO model from: %s and %s\n", weightsPath.c_str(), configPath.c_str());
    Net net;
    try {
        net = readNetFromDarknet(configPath, weightsPath);
        if (net.empty()) {
            std::cerr << "Failed to load YOLO network (net is empty)" << std::endl;
            return "Error: Failed to load YOLO network";
        }
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV exception while loading model: " << e.what() << std::endl;
        return "Error: OpenCV exception during model loading";
    } catch (const std::exception& e) {
        std::cerr << "Exception while loading model: " << e.what() << std::endl;
        return "Error: Exception during model loading";
    }

    printf("Model loaded successfully.\n");

        vector<String> layerNames = ctx.net.getLayerNames();
        vector<int> outLayers = ctx.net.getUnconnectedOutLayers();
        ctx.outputLayers.reserve(outLayers.size());
        for (int idx : outLayers) {
            ctx.outputLayers.push_back(layerNames[idx - 1]);
        }
    });
    return ctx;
}

const set<int>& vehicleClassIds() {
    static const set<int> ids = {2, 3, 5, 7};
    return ids;
}
} // namespace

// =================================================================
// === NEW FUNCTION: runDetection() — your old main() code goes here ===
// =================================================================
string analyzeTrafficDensity(const string& imagePath)
{
    // Load image
    Mat image = imread(imagePath);
    if (image.empty()) {
        cerr << "Image not found!" << endl;
        return "Error: Image not found";
    }

    printf("Image loaded successfully.\n");

    int height = image.rows;
    int width = image.cols;

    // Prepare input
    Mat blob;
    blobFromImage(image, blob, 0.00392, Size(416, 416),
                  Scalar(0, 0, 0), true, false);
    ctx.net.setInput(blob);

    vector<Mat> outs;
    ctx.net.forward(outs, ctx.outputLayers);

    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (auto& out : outs) {
        for (int i = 0; i < out.rows; i++) {
            float* data = (float*)out.ptr(i);
            Mat scores = out.row(i).colRange(5, out.cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            int classId = classIdPoint.x;

            if (confidence > 0.5 && vehicleIds.count(classId)) {
                int centerX = (int)(data[0] * width);
                int centerY = (int)(data[1] * height);
                int w = (int)(data[2] * width);
                int h = (int)(data[3] * height);
                int x = centerX - w / 2;
                int y = centerY - h / 2;

                boxes.push_back(Rect(x, y, w, h));
                confidences.push_back((float)confidence);
                classIds.push_back(classId);
            }
        }
    }

    vector<int> indexes;
    NMSBoxes(boxes, confidences, 0.5, 0.4, indexes);

    int vehicleCount = indexes.size();

    TrafficDensity densityAnalyzer(0.02);
    vector<Rect> finalBoxes;
    for (int idx : indexes) finalBoxes.push_back(boxes[idx]);
    double density = densityAnalyzer.computeDensity(finalBoxes, image);
    string condition = densityAnalyzer.analyzeDensity(density);

    std::string report = std::to_string(vehicleCount) + 
    " vehicles detected with density " + 
    std::to_string(density) + 
    ". Condition: " + 
    condition;

    // Draw boxes
    for (int idx : indexes) {
        Rect box = boxes[idx];
        drawRoundedRectangle(image, box, Scalar(0, 255, 0), 2);
        putText(image, "Vehicle",
                Point(box.x, box.y - 8),
                FONT_HERSHEY_SIMPLEX, 0.6,
                Scalar(0, 255, 0), 2);
    }

    static bool windowInitialized = false;
    if (!windowInitialized) {
        namedWindow("YOLO Vehicle Detection + Density", WINDOW_NORMAL);
        resizeWindow("YOLO Vehicle Detection + Density", 1280, 720);
        windowInitialized = true;
    }

    imshow("YOLO Vehicle Detection + Density", image);
    waitKey(1);   // non-blocking; window stays open

    return {report};

}
