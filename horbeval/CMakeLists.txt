cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(highorbit_evaluation)

if (MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

set(
	core_sources

	src/core/json.cpp
	src/core/SGP4.cpp
	src/core/observation.cpp
	src/core/highorbit.cpp
	src/core/geometry.cpp
	src/core/interface.cpp
)

set(
	gui_sources

	include/gui/mainview.h
	src/gui/mainview.cpp
	include/gui/pathview.h
	src/gui/pathview.cpp
	include/gui/mainmodel.h
	src/gui/mainmodel.cpp
	include/gui/tablemodel.h
	src/gui/tablemodel.cpp
	include/gui/guisettings.h
	src/gui/guisettings.cpp
	include/gui/plotview.h
	src/gui/plotview.cpp
    src/gui/guisrc.qrc
	src/gui/logger.cpp
)

add_executable(
	highorbit_evaluation

	src/main.cpp
	${core_sources}
	${gui_sources}
	
 )
target_include_directories(
	highorbit_evaluation 
	PRIVATE
	include/core include/gui
	${CMAKE_SOURCE_DIR}/external/wjson 
	${CMAKE_SOURCE_DIR}/external/wjson/external/faslib
)
target_link_libraries(
	highorbit_evaluation
	ballistic
	Qt5::Widgets
	Qt5::Charts
)
