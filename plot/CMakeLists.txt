cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(plot_library)

find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

add_library(libplot STATIC src/plot.cpp)
target_include_directories(libplot PUBLIC include)
target_link_libraries(libplot PRIVATE Qt5::Widgets Qt5::Charts)