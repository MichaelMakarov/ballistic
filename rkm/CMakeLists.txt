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
	src/core/models.cpp
	src/core/computation.cpp
	src/core/motion.cpp
	src/core/rotation.cpp
	src/core/forecast.cpp
	src/core/formatoutput.cpp	
	src/core/settings.cpp
)

set(
	gui_sources

	include/gui/mainview.hpp
	src/gui/mainview.cpp
	include/gui/pathview.hpp
	src/gui/pathview.cpp
	include/gui/mainmodel.hpp
	src/gui/mainmodel.cpp
	include/gui/datamodel.hpp
	src/gui/datamodel.cpp
	include/gui/presenter.hpp
	src/gui/presenter.cpp

	
	include/gui/gui.hpp
	src/gui/gui.cpp
    src/gui/guisrc.qrc
)

add_executable(
	kinematic_rotation

	src/main.cpp
	${core_sources}
	${gui_sources}
	
 )
target_include_directories(
	kinematic_rotation 
	PRIVATE
	include/core include/gui
	${CMAKE_SOURCE_DIR}/external/wjson 
	${CMAKE_SOURCE_DIR}/external/wjson/external/faslib
)
target_link_libraries(
	kinematic_rotation
	ballistic
	Qt5::Widgets
	Qt5::Charts
)
