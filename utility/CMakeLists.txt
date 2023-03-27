cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(utility_library)

add_library(libutility STATIC src/fileutility.cpp src/pathutility.cpp)
target_include_directories(libutility PUBLIC include)