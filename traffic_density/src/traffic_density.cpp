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
#include <nlohmann/json.hpp>  // JSON library (https://github.com/nlohmann/json)

using namespace cv;
using namespace dnn;
using namespace std;
using json = nlohmann::json;
using namespace std::chrono_literals;

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
        cout << "Estimated density: " << density << endl;
        if (density > threshold_) {
            cout << "⚠️  Heavy traffic detected!" << endl;
            return "Heavy traffic";
        } else {
            cout << "✅  Light traffic." << endl;
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

// === Get timestamp string ===
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

// === Save JSON Report ===
void saveReportJSON(int vehicleCount, double density, const string& condition) {
    json report;
    report["timestamp"] = getTimestamp();
    report["vehicle_count"] = vehicleCount;
    report["traffic_density"] = density;
    report["condition"] = condition;

    string filename = "traffic_report_" + getTimestamp() + ".json";
    ofstream file(filename);
    if (file.is_open()) {
        file << setw(4) << report;
        file.close();
        cout << "📝 Report saved to: " << filename << endl;
    } else {
        cerr << "❌ Failed to save JSON report!" << endl;
    }
}

int main() {
    // Load YOLO model
    Net net = readNet("../resources/models/yolov3.weights", "../resources/models/yolov3.cfg");

    // Vehicle classes (COCO indices)
    set<int> vehicleClassIds = {2, 3, 5, 7};

    // Load image
    Mat image = imread("../resources/images/gemini_sec_cam_traffic.png");
    if (image.empty()) {
        cerr << "Image not found!" << endl;
        return -1;
    }

    int height = image.rows;
    int width = image.cols;

    // Prepare input blob
    Mat blob;
    blobFromImage(image, blob, 0.00392, Size(416, 416), Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    // Get output layer names
    vector<String> layerNames = net.getLayerNames();
    vector<int> outLayers = net.getUnconnectedOutLayers();
    vector<String> outputLayers;
    for (int i : outLayers)
        outputLayers.push_back(layerNames[i - 1]);

    // Forward pass
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
                int centerX = static_cast<int>(data[0] * width);
                int centerY = static_cast<int>(data[1] * height);
                int w = static_cast<int>(data[2] * width);
                int h = static_cast<int>(data[3] * height);
                int x = centerX - w / 2;
                int y = centerY - h / 2;

                boxes.push_back(Rect(x, y, w, h));
                confidences.push_back((float)confidence);
                classIds.push_back(classId);
            }
        }
    }

    // Non-Maximum Suppression
    vector<int> indexes;
    NMSBoxes(boxes, confidences, 0.5, 0.4, indexes);

    int vehicleCount = (int)indexes.size();
    cout << "🚘 Detected vehicles: " << vehicleCount << endl;

    // Calculate density and condition
    TrafficDensity densityAnalyzer(0.02);  // 2% threshold
    vector<Rect> finalBoxes;
    for (int idx : indexes) finalBoxes.push_back(boxes[idx]);
    double density = densityAnalyzer.computeDensity(finalBoxes, image);
    string condition = densityAnalyzer.analyzeDensity(density);

    // Save JSON report
    saveReportJSON(vehicleCount, density, condition);

    // Draw results
    for (size_t i = 0; i < indexes.size(); i++) {
        int idx = indexes[i];
        Rect box = boxes[idx];
        drawRoundedRectangle(image, box, Scalar(0, 255, 0), 2);
        putText(image, "Vehicle " + to_string(i + 1),
                Point(box.x, box.y - 8),
                FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
    }

    imshow("YOLO Vehicle Detection + Density", image);
    waitKey(0);
    destroyAllWindows();

    return 0;
}
