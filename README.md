# Security Camera Image Detector

A real-time traffic analysis system that monitors vehicle density using images from public security cameras near UFABC (Santo André, SP). The system uses YOLOv3 for vehicle detection and provides automated traffic density reports.

## 🚀 Features

- **Real-time Image Capture**: Downloads frames from public camera streams
- **Vehicle Detection**: Uses YOLOv3 neural network via OpenCV DNN
- **Traffic Density Analysis**: Calculates density based on detected vehicle area
- **Image Pre-processing**: CLAHE enhancement and bilateral filtering
- **Automated Reports**: Generates JSON traffic reports with timestamps
- **Visual Feedback**: Displays annotated images with detected vehicles

## 📋 Requirements

- **C++17** or higher
- **CMake** 3.10+
- **OpenCV** 4.12.0+ with modules: `core`, `highgui`, `imgproc`, `dnn`
- **nlohmann/json** for JSON serialization
- **HDF5** and **VTK** (OpenCV dependencies)

## 🛠️ Installation

### 1. Clone the Repository
```bash
git clone https://github.com/brMonteiro-G/security-cam-image-detector.git
cd security-cam-image-detector
```

### 2. Download YOLO Model Files
Download the YOLOv3 weights and config files:
```bash
cd traffic_density/resources/models/

# Download weights (~237MB)
wget https://pjreddie.com/media/files/yolov3.weights

# Download config (~8KB)
wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3.cfg
```

### 3. Build the Project
```bash
cd traffic_density/build
cmake ..
make
```

## 🚦 Usage

Run the traffic analysis application:
```bash
cd traffic_density/build
./main_exec
```

The application will:
1. Capture a frame from the camera stream
2. Apply image pre-processing (CLAHE + bilateral filter)
3. Detect vehicles using YOLOv3
4. Calculate traffic density
5. Display annotated image and print report

### Output Example
```
Loading YOLO model from: /path/to/yolov3.weights
YOLO model loaded successfully!
Detected 28 vehicles...
Traffic Report for avenida_dos_estados:
{
  "avenue": "avenida_dos_estados",
  "timestamp": "2025-11-22T14:30:00",
  "vehicle_count": 28,
  "density": 0.169,
  "status": "Heavy traffic"
}
```

## 📁 Project Structure

```
security-cam-image-detector/
├── traffic_density/
│   ├── main.cpp                          # Pipeline orchestrator
│   ├── CMakeLists.txt                    # Build configuration
│   ├── build/                            # Build artifacts
│   │   └── main_exec                     # Executable
│   ├── src/
│   │   ├── Input/
│   │   │   └── ingest.cpp                # Camera frame capture
│   │   └── service/
│   │       ├── pre_processing/
│   │       │   └── filter_image.cpp      # CLAHE + bilateral filter
│   │       └── processing/
│   │           └── traffic_density.cpp   # YOLO detection & analysis
│   ├── include/
│   │   └── traffic_density.hpp           # TrafficDensity class header
│   ├── resources/
│   │   ├── images/                       # Captured & processed images
│   │   │   └── avenida_dos_estados/
│   │   └── models/                       # YOLO model files
│   │       ├── yolov3.weights
│   │       └── yolov3.cfg
│   └── utils/
│       └── debug_frame.cpp               # Debug visualization tools
├── .github/
│   └── copilot-instructions.md           # AI agent documentation
├── ARCHITECTURE.md                       # Detailed architecture diagrams
└── README.md                             # This file
```

## 🔧 Architecture

The system follows a 4-step pipeline:

1. **Image Ingestion** (`ingest_camera()`)
   - Captures frames from Santo André SP public camera
   - Saves timestamped images to `resources/images/`
   - Returns `{avenueName, imagePath}` pair

2. **Pre-processing** (`test_static_image()`)
   - Applies CLAHE (Contrast Limited Adaptive Histogram Equalization)
   - Applies bilateral filter for noise reduction
   - Saves filtered image

3. **Traffic Analysis** (`analyzeTrafficDensity()`)
   - Loads YOLOv3 model using `readNetFromDarknet()`
   - Detects vehicles with 0.5 confidence threshold
   - Applies Non-Maximum Suppression (NMS)
   - Computes density: `vehicle_area / frame_area`
   - Generates JSON report

4. **Output & Notification** (future)
   - Displays annotated image with bounding boxes
   - Saves JSON traffic report
   - *(Future)* Sends SNS notifications for alerts

For detailed architecture diagrams, see [ARCHITECTURE.md](ARCHITECTURE.md).

## ⚙️ Configuration

### Camera Source
Edit `src/Input/ingest.cpp` to change the camera URL:
```cpp
string video_url = "http://camera.domain/stream?id=655";
string avenue_name = "your_avenue_name";
```

### YOLO Parameters
Edit `src/service/processing/traffic_density.cpp`:
```cpp
float confThreshold = 0.5;  // Confidence threshold
float nmsThreshold = 0.4;   // NMS threshold
```

### Traffic Density Thresholds
```cpp
if (density > 0.15) return "Heavy traffic";
else if (density > 0.08) return "Moderate traffic";
else return "Light traffic";
```

## 🐛 Troubleshooting

### Segmentation Fault on Model Loading
**Problem**: Application crashes with exit code 139.

**Solution**: Ensure OpenCV DNN module is linked in `CMakeLists.txt`:
```cmake
find_package(OpenCV REQUIRED core highgui imgproc dnn)
```
Also use the Darknet-specific loader:
```cpp
// ✅ CORRECT
cv::dnn::Net net = cv::dnn::readNetFromDarknet(configPath, weightsPath);

// ❌ WRONG (causes segfault)
cv::dnn::Net net = cv::dnn::readNet(weightsPath, configPath);
```

### Model Files Not Found
Ensure YOLO files are in the correct location:
```bash
ls -lh traffic_density/resources/models/
# Should show:
# yolov3.weights (~237MB)
# yolov3.cfg (~8KB)
```

### Build Errors
Clean and rebuild:
```bash
cd traffic_density/build
rm -rf *
cmake ..
make
```

## 📊 Performance

- **Frame Processing**: ~2-3 seconds per image (depends on hardware)
- **Model Loading**: ~1-2 seconds (one-time initialization)
- **Detection Accuracy**: YOLOv3 provides ~80% mAP on COCO dataset
- **Supported Vehicles**: Cars, trucks, buses, motorcycles

## 🔮 Future Enhancements

- [ ] AWS SNS notification integration
- [ ] Continuous monitoring mode (loop execution)
- [ ] Multi-camera support
- [ ] Database storage for historical data
- [ ] Web dashboard for real-time monitoring
- [ ] REST API for external integrations

## 📝 License

This project is for educational purposes as part of UFABC research.

## 👥 Contributors

- Gabriel Monteiro ([@brMonteiro-G](https://github.com/brMonteiro-G))

## 📧 Contact

For questions or support, please open an issue on GitHub.

---

**Build Command**: `cd traffic_density/build && cmake .. && make && ./main_exec`