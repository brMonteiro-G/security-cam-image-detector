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
#include <nlohmann/json.hpp>

using namespace cv;
using namespace dnn;
using namespace std;
using json = nlohmann::json;

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


// =================================================================
// === NEW FUNCTION: runDetection() — your old main() code goes here ===
// =================================================================
string analyzeTrafficDensity(const string& imagePath)
{
    printf("Analyzing traffic density for image at %s\n", imagePath.c_str());
    // Load YOLO
    Net net = readNet("../../../resources/models/yolov3.weights",
                      "../../../resources/models/yolov3.cfg");

    set<int> vehicleClassIds = {2, 3, 5, 7};

    printf("YOLO model loaded.\n");
    // Load image
    Mat image = imread(imagePath);
    if (image.empty()) {
        return "Error: could not load image at " + imagePath;
    }

    int height = image.rows;
    int width = image.cols;


    printf("Preparing input blob for YOLO.\n");
    // Prepare input
    Mat blob;
    blobFromImage(image, blob, 0.00392, Size(416, 416),
                  Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    // Output layers
    vector<String> layerNames = net.getLayerNames();
    vector<int> outLayers = net.getUnconnectedOutLayers();
    vector<String> outputLayers;
    for (int i : outLayers)
        outputLayers.push_back(layerNames[i - 1]);

    vector<Mat> outs;
    net.forward(outs, outputLayers);

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

            if (confidence > 0.5 && vehicleClassIds.count(classId)) {
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

    string report =
        to_string(vehicleCount) +
        " vehicles detected. Density = " +
        to_string(density) +
        ". Condition: " + condition;

    // Draw boxes
    for (int idx : indexes) {
        Rect box = boxes[idx];
        drawRoundedRectangle(image, box, Scalar(0, 255, 0), 2);
        putText(image, "Vehicle",
                Point(box.x, box.y - 8),
                FONT_HERSHEY_SIMPLEX, 0.6,
                Scalar(0, 255, 0), 2);
    }

    imshow("YOLO Vehicle Detection + Density", image);
    waitKey(1);   // non-blocking; window stays open

    return report;
}
