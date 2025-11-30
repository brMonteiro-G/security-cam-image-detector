cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# =======================
# Find Dependencies
# =======================

# OpenCV
find_package(OpenCV)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# =======================
# Include Directories
# =======================
include_directories(
    ${OpenCV_INCLUDE_DIRS}
)

# =======================
# Main Executable
# =======================
add_executable(main_exec
    main.cpp
    src/service/processing/traffic_density.cpp
    src/service/pre_processing/filter_image.cpp
    src/Input/ingest.cpp
)

# =======================
# Link Libraries
# =======================
target_link_libraries(main_exec
    ${OpenCV_LIBS}          # OpenCV core libraries
    nlohmann_json::nlohmann_json
)
