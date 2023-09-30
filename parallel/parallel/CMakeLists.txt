message(STATUS "$processing source directory ${CMAKE_CURRENT_SOURCE_DIR}")

add_library(queue_manager STATIC src/parallel.cpp)
target_include_directories(queue_manager PUBLIC include)