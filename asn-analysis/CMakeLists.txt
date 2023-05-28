cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

project(asn_analysis)


add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/measurement.cpp
    src/model.cpp
    src/forecast.cpp
    src/computation.cpp
    src/config.cpp
    src/initgpt.cpp
    src/timer.cpp
    src/figure.cpp
    include/figure.hpp
    src/atmosphere.cpp
    src/spaceweather.cpp
    src/csvutility.cpp
    src/timeutility.cpp
    src/geometry.cpp
    src/rotator.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(
    ${PROJECT_NAME}
    mathlib
    ballistic
    liburl
    libutility
    pugixml
	Qt5::Widgets
	Qt5::Charts
)