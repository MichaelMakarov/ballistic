cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ballistic_library)

add_subdirectory(maths)
add_subdirectory(times)
add_subdirectory(ballistic)