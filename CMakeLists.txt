cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(ballistic_projects)

## getting all subdirectories in external
set(extdir ${CMAKE_CURRENT_SOURCE_DIR}/external)
FILE(GLOB required_submodules RELATIVE ${extdir} ${extdir}/*)
message(STATUS "checking and initializing submodules")
find_package(Git QUIET)
if (GIT_FOUND)
    foreach (submodule ${required_submodules})
        set(submodule_path ${extdir}/${submodule})
        if (NOT EXISTS ${submodule_path}/CMakeLists.txt)
            message(STATUS "init ${submodule_path}")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- ${submodule_path} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endif()
        add_subdirectory(${submodule_path})
    endforeach()
endif()

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)

include(scripts/functions.cmake)

subdirs(
    ballistic
    utility
    urlproc
    asndataproc
    plot
)



if (MSVC)
    add_compile_options(_CRT_SECURE_NO_WARNINGS)
endif()