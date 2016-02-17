cmake_minimum_required(VERSION 2.8)
project(HashCode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DsrcPath="${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_FILES main.cpp draw_img.cpp knapsack.cpp)
add_executable(HashCode ${SOURCE_FILES})
