cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# Option to enable AWS SNS support
option(ENABLE_AWS_SNS "Enable AWS SNS notifications" OFF)

# Option to enable GUI (highgui) - disable for Lambda
option(ENABLE_GUI "Enable OpenCV highgui for GUI display" ON)

# =======================
# Find Dependencies
# =======================

# OpenCV - highgui only needed for local development with GUI
if(ENABLE_GUI)
    find_package(OpenCV REQUIRED core highgui imgproc dnn videoio)
    find_package(HDF5 REQUIRED)
    find_package(VTK REQUIRED)
    find_package(OpenGL REQUIRED)
else()
    find_package(OpenCV REQUIRED core imgproc imgcodecs dnn videoio)
endif()

# nlohmann_json - header-only library (no find_package needed)
# Just ensure /usr/local/include is in include path

# AWS SDK (optional)
if(ENABLE_AWS_SNS)
    find_package(AWSSDK REQUIRED COMPONENTS sns core)
    add_definitions(-DUSE_AWS_SNS)
    message(STATUS "AWS SNS support enabled")
endif()

# =======================
# Include Directories
# =======================
if(ENABLE_GUI)
    include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${HDF5_INCLUDE_DIRS}
        /usr/local/include
        include
    )
else()
    include_directories(
        ${OpenCV_INCLUDE_DIRS}
        /usr/local/include
        include
    )
endif()

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
if(ENABLE_GUI)
    target_link_libraries(main_exec
        ${OpenCV_LIBS}
        ${HDF5_LIBRARIES}
        ${VTK_LIBRARIES}
        OpenGL::GL
        OpenGL::GLU
    )
else()
    target_link_libraries(main_exec
        ${OpenCV_LIBS}
    )
endif()

# Link AWS SDK if enabled
if(ENABLE_AWS_SNS)
    target_link_libraries(main_exec ${AWSSDK_LINK_LIBRARIES})
endif()