# =======================
# Main Executable
# =======================
add_executable(main_exec main.cpp)
target_link_libraries(main_exec
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    ${AWSSDK_LINK_LIBRARIES}
)
cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# =======================
# Find Dependencies
# =======================

# OpenCV
find_package(OpenCV REQUIRED)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# AWS SDK
# Make sure AWS SDK is installed and built with SNS component enabled.
# You can install AWS SDK using vcpkg or build from source.
find_package(AWSSDK REQUIRED COMPONENTS sns core)

# =======================
# Include Directories
# =======================
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${AWSSDK_INCLUDE_DIRS}
    include
)

# =======================
# Project Executable
# =======================
add_executable(traffic_density
    src/traffic_density.cpp
    src/notification.cpp
)

# =======================
# Link Libraries
# =======================
target_link_libraries(traffic_density
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    ${AWSSDK_LINK_LIBRARIES}
)