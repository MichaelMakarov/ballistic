cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(plot_library)

add_library(libplot STATIC src/plot.cpp)
target_include_directories(libplot PUBLIC include)