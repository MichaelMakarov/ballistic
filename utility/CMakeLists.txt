cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(utility_library)

add_library(
    libutility STATIC 
    src/fileutils.cpp 
    src/printutils.cpp
)
target_include_directories(libutility PUBLIC include)