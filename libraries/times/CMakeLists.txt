cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(times_library)
add_library(
	times STATIC
	src/times.cpp
)
target_include_directories(
	times PUBLIC include
)
