message(STATUS "$processing source directory ${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(example_qm example.cpp)
target_link_libraries(example_qm PUBLIC queue_manager)