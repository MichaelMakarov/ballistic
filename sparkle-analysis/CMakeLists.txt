qt5_add_resources(qrc_sources src/guisrc.qrc)
add_executable( 
	sparkle_analysis
	src/main.cpp
	
	# core sources
	src/json.cpp
	src/SGP4.cpp
	src/tle.cpp
	src/models.cpp
	src/interval.cpp
	src/motion.cpp
	src/logger.cpp
	src/forecast.cpp
	src/settings.cpp

	#gui sources
	src/mainview.cpp
	src/pathview.cpp
	src/mainmodel.cpp
	src/table_view.cpp
	src/tree_view.cpp
	src/residual_graphic.cpp
	src/gui.cpp
    
	#qrc sources
	${qrc_sources}	
 )
target_include_directories(sparkle_analysis PRIVATE include)
target_link_libraries(	
	sparkle_analysis PRIVATE 
	ballistic 
	json 
	utility 
	mathlib 
	parallel
	graphic
	async
	Qt5::Widgets
)
