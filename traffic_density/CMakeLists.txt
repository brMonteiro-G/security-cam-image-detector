cmake_minimum_required(VERSION 3.10)
project(TrafficDensity)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(traffic_density src/main.cpp src/traffic_density.cpp)
target_link_libraries(traffic_density ${OpenCV_LIBS})
