# Copilot Instructions for security-cam-image-detector

## Project Overview
- This project analyzes traffic density using images from security cameras near UFABC.
- Main components:
  - **Image Ingestion**: Downloads and saves frames from a public camera stream (`src/Input/ingest.cpp`).
  - **Traffic Density Analysis**: Processes images to estimate vehicle density using OpenCV and DNN (`src/service/processing/traffic_density.cpp`, `include/traffic_density.hpp`).
  - **Utilities**: Includes helpers for debugging and visualization (e.g., `utils/debug_frame.cpp`).

## Architecture & Data Flow
- Images are periodically captured from a video stream and saved to `resources/images/`.
- Analysis modules read these images, detect vehicles, and compute density metrics.
- Results (e.g., density, alerts) can be output to console or saved as JSON reports.

## Build & Run
- Uses CMake for build configuration (`CMakeLists.txt`).
- Requires OpenCV and nlohmann/json as dependencies.
- Build from the `traffic_density` directory:
  ```sh
  mkdir -p build && cd build
  cmake .. && make
  ./traffic_density
  ```
- For image ingestion, run the binary built from `src/Input/ingest.cpp`.

## Conventions & Patterns
- All image processing uses OpenCV (`cv::Mat`, `cv::VideoCapture`, etc.).
- Traffic density is computed as the ratio of detected vehicle area to total frame area.
- JSON output uses nlohmann/json.
- Debugging visuals (e.g., grid overlays) are implemented in utility files.
- Place new models/configs in `resources/models/`.

## Key Files & Directories
- `src/Input/ingest.cpp`: Camera frame downloader.
- `src/service/processing/traffic_density.cpp`: Core traffic density logic.
- `include/traffic_density.hpp`: TrafficDensity class interface.
- `utils/debug_frame.cpp`: Grid and debug overlays.
- `resources/images/`: Saved camera frames.
- `resources/models/`: Model configs/weights (e.g., YOLO).

## Tips for AI Agents
- Always check for required directories before saving files.
- Use CMake for all builds; do not invoke g++ directly.
- Follow the existing class and function naming patterns.
- Prefer OpenCV and nlohmann/json for new features.
- Place new source files in the appropriate subdirectory (Input, service/processing, utils, etc.).
