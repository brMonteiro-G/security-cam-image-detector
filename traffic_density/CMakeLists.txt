cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# =======================
# Find Dependencies
# =======================

# OpenCV
find_package(OpenCV REQUIRED core highgui imgproc dnn)
find_package(HDF5 REQUIRED)

# Find VTK
find_package(VTK REQUIRED)
if(VTK_FOUND)
    include(${VTK_USE_FILE})
endif()

# nlohmann_json
find_package(nlohmann_json REQUIRED)

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
)