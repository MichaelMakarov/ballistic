cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(urlproc_library)

add_library(liburlproc STATIC src/urlproc.cpp)
target_include_directories(liburlproc PUBLIC include)
target_link_libraries(liburlproc PRIVATE libcurl PRIVATE libutility)
