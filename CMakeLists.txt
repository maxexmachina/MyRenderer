cmake_minimum_required(VERSION 3.9)
project(MyRenderer)

if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -g -pg -O3")

add_executable(MyRenderer src/main.cpp dependencies/tgaimage.cpp dependencies/tgaimage.h src/model.cpp src/model.h src/geometry.h dependencies/fisqrt.h dependencies/fisqrt.cpp)
