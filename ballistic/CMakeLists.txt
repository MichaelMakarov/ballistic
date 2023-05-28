cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ballistic_library)

add_library(
	ballistic STATIC
	src/jd.cpp
	src/gpt.cpp
	src/transform.cpp
	src/sunmoon.cpp	
)
target_include_directories(ballistic PUBLIC include)
target_link_libraries(ballistic PUBLIC mathlib queue_manager)