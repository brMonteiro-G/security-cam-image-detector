# Security Camera Image Detector - Architecture

## System Architecture

```mermaid
graph TB
    subgraph "Entry Point"
        Main[main.cpp<br/>Pipeline Orchestrator]
    end

    subgraph "Image Ingestion"
        Camera[Camera Stream<br/>Santo André, SP]
        IngestFunc[ingest_camera&#40;&#41;<br/>src/Input/ingest.cpp]
        CameraImages[(resources/images/<br/>avenida_dos_estados/)]
    end

    subgraph "Pre-Processing"
        PreProcess[test_static_image&#40;&#41;<br/>src/service/pre_processing/<br/>filter_image.cpp]
        CLAHE[CLAHE Enhancement]
        Bilateral[Bilateral Filter]
        ProcessedImages[(Filtered Images<br/>resources/images/)]
    end

    subgraph "Traffic Analysis"
        Analyze[analyzeTrafficDensity&#40;&#41;<br/>src/service/processing/<br/>traffic_density.cpp]
        YOLOModel[(YOLO Model<br/>resources/models/<br/>yolov3.weights<br/>yolov3.cfg)]
        DetectVehicles[Vehicle Detection<br/>OpenCV DNN]
        ComputeDensity[Density Calculation<br/>TrafficDensity Class]
    end

    subgraph "Output"
        Report[Traffic Report<br/>String Output]
        Display[OpenCV Display<br/>imshow&#40;&#41;]
        Notification[sendTrafficNotification<br/>&#40;future&#41;]
    end

    Camera -->|Video Stream| IngestFunc
    IngestFunc -->|Capture Frame| CameraImages
    IngestFunc -->|pair&#60;avenue, path&#62;| Main
    
    Main -->|imagePath, avenueName| PreProcess
    PreProcess -->|CLAHE| CLAHE
    CLAHE -->|Enhanced| Bilateral
    Bilateral -->|Filtered| ProcessedImages
    PreProcess -->|processedPath| Main
    
    Main -->|processedPath, avenueName| Analyze
    YOLOModel -->|Load Model| Analyze
    Analyze -->|readNetFromDarknet| DetectVehicles
    DetectVehicles -->|Bounding Boxes| ComputeDensity
    ComputeDensity -->|Density Metrics| Report
    
    Report --> Main
    Report -.->|Future| Notification
    Analyze -->|Annotated Image| Display

    style Main fill:#e1f5ff
    style IngestFunc fill:#fff4e1
    style PreProcess fill:#ffe1f5
    style Analyze fill:#e1ffe1
    style YOLOModel fill:#ffe1e1
```

## Data Flow

```mermaid
sequenceDiagram
    participant M as main.cpp
    participant I as ingest_camera()
    participant P as test_static_image()
    participant A as analyzeTrafficDensity()
    participant Y as YOLOv3 Model
    participant D as Display

    M->>I: Call ingest_camera()
    I->>I: Capture from camera stream
    I->>I: Save to resources/images/
    I-->>M: Return (avenueName, imagePath)
    
    M->>P: test_static_image(imagePath, avenueName)
    P->>P: Apply CLAHE + Bilateral Filter
    P->>P: Save filtered image
    P-->>M: Return processedImagePath
    
    M->>A: analyzeTrafficDensity(processedPath, avenueName)
    A->>Y: Load readNetFromDarknet()
    Y-->>A: Neural Network loaded
    A->>A: Read image with imread()
    A->>A: Prepare blob (416x416)
    A->>Y: Forward pass (detect vehicles)
    Y-->>A: Detection results (boxes, confidences)
    A->>A: NMS (Non-Maximum Suppression)
    A->>A: Compute density (vehicle_area/frame_area)
    A->>A: Generate report string
    A->>D: Draw boxes + imshow()
    A-->>M: Return report
    
    M->>M: Log report
    Note over M: Future: sendTrafficNotification()
```

## Folder Structure

```mermaid
graph TD
    Root[security-cam-image-detector/]
    
    Root --> TD[traffic_density/]
    Root --> GH[.github/]
    Root --> README[README.md]
    
    GH --> Instructions[copilot-instructions.md]
    
    TD --> MainCpp[main.cpp]
    TD --> CMake[CMakeLists.txt]
    TD --> Build[build/]
    TD --> Src[src/]
    TD --> Include[include/]
    TD --> Resources[resources/]
    TD --> Utils[utils/]
    
    Build --> Exec[main_exec]
    Build --> BuildFiles[CMake artifacts]
    
    Src --> Input[Input/]
    Src --> Service[service/]
    
    Input --> Ingest[ingest.cpp]
    
    Service --> PreProc[pre_processing/]
    Service --> Processing[processing/]
    Service --> PostProc[post_processing/]
    Service --> Notif[notification/]
    
    PreProc --> Filter[filter_image.cpp]
    Processing --> Traffic[traffic_density.cpp]
    
    Include --> Headers[traffic_density.hpp]
    
    Resources --> Images[images/]
    Resources --> Models[models/]
    
    Images --> Avenue[avenida_dos_estados/<br/>screenshot_*.jpg]
    Images --> Filtered[filtered_image_*.png]
    
    Models --> Weights[yolov3.weights<br/>~237MB]
    Models --> Config[yolov3.cfg<br/>~8KB]
    
    Utils --> Debug[debug_frame.cpp]

    style Root fill:#e1f5ff
    style TD fill:#fff4e1
    style Src fill:#ffe1f5
    style Resources fill:#e1ffe1
    style Models fill:#ffe1e1
    style Build fill:#f0f0f0
```

## Module Dependencies

```mermaid
graph LR
    subgraph "Core Modules"
        Main[main.cpp]
        Ingest[ingest.cpp]
        Filter[filter_image.cpp]
        Traffic[traffic_density.cpp]
    end
    
    subgraph "External Dependencies"
        OpenCV[OpenCV 4.12+<br/>core, highgui,<br/>imgproc, dnn]
        JSON[nlohmann/json]
        HDF5[HDF5]
        VTK[VTK]
        FS[C++17 filesystem]
    end
    
    subgraph "Resources"
        YOLOWeights[yolov3.weights]
        YOLOConfig[yolov3.cfg]
        Images[Camera Images]
    end
    
    Main --> Ingest
    Main --> Filter
    Main --> Traffic
    
    Ingest --> OpenCV
    Ingest --> FS
    Ingest --> Images
    
    Filter --> OpenCV
    Filter --> FS
    
    Traffic --> OpenCV
    Traffic --> YOLOWeights
    Traffic --> YOLOConfig
    Traffic --> JSON
    Traffic --> FS
    
    OpenCV -.depends on.-> HDF5
    OpenCV -.depends on.-> VTK

    style Main fill:#e1f5ff
    style OpenCV fill:#ffe1e1
    style YOLOWeights fill:#fff4e1
    style YOLOConfig fill:#fff4e1
```

## Build Process

```mermaid
flowchart TB
    Start([Developer]) --> CD[cd traffic_density/build]
    CD --> CMake[cmake ..]
    
    CMake --> FindDeps{Find<br/>Dependencies}
    FindDeps -->|OpenCV| CheckDNN{DNN Module<br/>Available?}
    FindDeps -->|JSON| CheckJSON[nlohmann_json]
    FindDeps -->|HDF5| CheckHDF5[HDF5 libs]
    FindDeps -->|VTK| CheckVTK[VTK libs]
    
    CheckDNN -->|Yes| ConfigOK[Configure Done]
    CheckDNN -->|No| Error[❌ Build Fails<br/>Segfault at runtime]
    
    CheckJSON --> ConfigOK
    CheckHDF5 --> ConfigOK
    CheckVTK --> ConfigOK
    
    ConfigOK --> Make[make]
    Make --> Compile[Compile .cpp files]
    
    Compile --> LinkMain[Link main.cpp]
    Compile --> LinkIngest[Link ingest.cpp]
    Compile --> LinkFilter[Link filter_image.cpp]
    Compile --> LinkTraffic[Link traffic_density.cpp]
    
    LinkMain --> LinkLibs[Link Libraries]
    LinkIngest --> LinkLibs
    LinkFilter --> LinkLibs
    LinkTraffic --> LinkLibs
    
    LinkLibs --> Executable[✅ main_exec]
    Error --> Fix[Add dnn to<br/>CMakeLists.txt]
    Fix --> CMake

    style Start fill:#e1f5ff
    style Executable fill:#e1ffe1
    style Error fill:#ffe1e1
    style ConfigOK fill:#fff4e1
```

## Key Patterns

### Function Signature Pattern
```cpp
// All processing functions follow this pattern:
std::string functionName(const std::string& imagePath, 
                         const std::string& avenueName);

// Ingestion returns structured data:
std::pair<std::string, std::string> ingest_camera();
// Returns: {avenueName, imagePath}
```

### Error Handling Pattern
```cpp
// Always check file existence
if (!std::filesystem::exists(path)) {
    std::cerr << "File not found: " << path << std::endl;
    return "Error: ...";
}

// Always validate cv::Mat
cv::Mat image = cv::imread(path);
if (image.empty()) {
    std::cerr << "Failed to load image: " << path << std::endl;
    return "Error: ...";
}
```

### Model Loading Pattern
```cpp
// ✅ CORRECT - Use Darknet-specific loader
cv::dnn::Net net = cv::dnn::readNetFromDarknet(configPath, weightsPath);

// ❌ WRONG - Generic loader causes segfault
// cv::dnn::Net net = cv::dnn::readNet(weightsPath, configPath);
```
