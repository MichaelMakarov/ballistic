cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(url_library)

add_library(liburl STATIC src/urlproc.cpp)
target_include_directories(liburl PUBLIC include)
target_link_libraries(liburl PRIVATE CURL::libcurl PRIVATE libutility)
