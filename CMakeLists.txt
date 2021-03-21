cmake_minimum_required(VERSION 3.17)
project(MyRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -g -pg -O3")

add_executable(MyRenderer src/main.cpp dependencies/tgaimage.cpp dependencies/tgaimage.h src/model.cpp src/model.h src/geometry.h dependencies/fisqrt.h dependencies/fisqrt.cpp)