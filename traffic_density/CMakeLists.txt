cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)

# Detect OpenCV depending on platform
if (WIN32)
    # If using vcpkg, point to your vcpkg toolchain
    # cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
endif()

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(traffic_density src/main.cpp src/traffic_density.cpp)
target_link_libraries(traffic_density ${OpenCV_LIBS})
