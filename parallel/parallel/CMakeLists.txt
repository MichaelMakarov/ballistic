message(STATUS "$processing source directory ${CMAKE_CURRENT_SOURCE_DIR}")

add_library(parallel STATIC src/parallel.cpp)
target_include_directories(parallel PUBLIC include)