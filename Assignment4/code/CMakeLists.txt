cmake_minimum_required(VERSION 3.10)
project(BezierCurve)

set(CMAKE_CXX_STANDARD 14)

set(OpenCV_DIR D:/OpenCV/opencv/mingw_build)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(BezierCurve main.cpp)
target_link_libraries(BezierCurve ${OpenCV_LIBRARIES})
