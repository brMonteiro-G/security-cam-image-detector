cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# Option to enable AWS SNS support
option(ENABLE_AWS_SNS "Enable AWS SNS notifications" OFF)

# =======================
# Find Dependencies
# =======================

# OpenCV
find_package(OpenCV REQUIRED core highgui imgproc dnn)
find_package(HDF5 REQUIRED)

# Find VTK (no longer using deprecated VTK_USE_FILE)
find_package(VTK REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# AWS SDK (optional)
if(ENABLE_AWS_SNS)
    find_package(AWSSDK REQUIRED COMPONENTS sns core)
    add_definitions(-DUSE_AWS_SNS)
    message(STATUS "AWS SNS support enabled")
endif()

# =======================
# Include Directories
# =======================
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIRS}    # Added HDF5 headers
    include
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
    ${HDF5_LIBRARIES}       # Added HDF5 libraries
    ${VTK_LIBRARIES}        # VTK libraries required for OpenCV viz
    OpenGL::GL
    OpenGL::GLU
)

# Link AWS SDK if enabled
if(ENABLE_AWS_SNS)
    target_link_libraries(main_exec ${AWSSDK_LINK_LIBRARIES})
endif()