cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(mathlibrary LANGUAGES CXX)

message(STATUS "$processing source directory ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(mathlib)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	add_subdirectory(example)
endif()
