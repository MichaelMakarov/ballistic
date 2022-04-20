cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ballistic_projects)

include(scripts/functions.cmake)

set(required_submodules wjson)
update_submodules(${required_submodules} --init --recursive)

add_subdirectory(libraries)
add_subdirectory(projects)
