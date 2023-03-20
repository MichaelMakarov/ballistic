set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

add_executable(
    csv_kinematic_rotation
    src/main.cpp
    src/measurement.cpp
    src/model.cpp
    src/forecast.cpp
    src/computation.cpp
    src/fileutility.cpp
    src/config.cpp
    src/initgpt.cpp
    src/timer.cpp
    src/figure.cpp
    include/figure.hpp
    src/atmosphere.cpp
    src/spaceweather.cpp
    src/csvutility.cpp
)
target_include_directories(
    csv_kinematic_rotation
    PUBLIC
    include
)
target_link_libraries(
    csv_kinematic_rotation
    PUBLIC
    mathlib
    ballistic
    libcurl
	Qt5::Widgets
	Qt5::Charts
)
#copydllfiles(csv_kinematic_rotation)