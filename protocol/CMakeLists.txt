cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(protocl_library)

add_library(protocol STATIC src/protocol.cpp)
target_include_directories(protocol PUBLIC include)
target_link_libraries(protocol PRIVATE libutility)
