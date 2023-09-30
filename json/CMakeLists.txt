cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(json_library)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yas/include)
    message(STATUS "init ${CMAKE_CURRENT_SOURCE_DIR}/yas")
    update_submodule(yas --init)
endif()

add_library(jsonlib INTERFACE)
target_include_directories(jsonlib INTERFACE yas/include)