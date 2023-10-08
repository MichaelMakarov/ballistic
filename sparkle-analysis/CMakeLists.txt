set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(
	core_sources
	src/core/json.cpp
	src/core/SGP4.cpp
	src/core/tle.cpp
	src/core/models.cpp
	src/core/interval.cpp
	src/core/motion.cpp
	# src/core/rotation.cpp
	src/core/forecast.cpp
	# src/core/formatoutput.cpp	
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
	# include/gui/presenter.hpp
	# src/gui/presenter.cpp	
	include/gui/gui.hpp
	src/gui/gui.cpp
    src/gui/guisrc.qrc
)

add_executable( 
	sparkle_analysis
	src/main.cpp
	${core_sources}
	${gui_sources}
	
 )
target_include_directories(sparkle_analysis PRIVATE include/core include/gui)
target_link_libraries(	
	sparkle_analysis PRIVATE 
	ballistic 
	json 
	utility 
	mathlib 
	parallel
	Qt5::Widgets
)
