if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yas/include)
    update_submodule(yas --init)
endif()

add_library(json INTERFACE)
target_include_directories(json INTERFACE yas/include include)
target_link_libraries(json INTERFACE utility)