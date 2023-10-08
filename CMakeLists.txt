cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(ballistic_projects)

find_package(CURL REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

include(scripts/functions.cmake)

update_cmake_submodule(pugixml --init)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)


add_subdirectory(ballistic)
add_subdirectory(utility)
add_subdirectory(url)
add_subdirectory(json)
add_subdirectory(asn-analysis)
add_subdirectory(sparkle-analysis)
add_subdirectory(protocol)
add_subdirectory(pugixml)
add_subdirectory(parallel)
add_subdirectory(mathlib)
add_subdirectory(graphic)