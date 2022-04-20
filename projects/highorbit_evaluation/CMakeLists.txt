if (MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

QT5_WRAP_CPP(
	moc_files
	include/mainwindow.h
)

add_executable(
	highorbit_evaluation

	src/highorbit_models.cpp
	src/celestrack_managing.cpp
	src/json_utils.cpp
	src/model_wrapper.cpp
	src/geometry_models.cpp
	src/SGP4.cpp
	src/main.cpp
	src/structure_types.cpp

	src/mainwindow.cpp
 "include/viewmodel.h" "src/viewmodel.cpp")
target_include_directories(
	highorbit_evaluation 
	PRIVATE 
	include 
	${CMAKE_SOURCE_DIR}/external/wjson 
	${CMAKE_SOURCE_DIR}/external/wjson/external/faslib
)
target_link_libraries(
	highorbit_evaluation
	ballistic
	Qt5::Widgets
	Qt5::Charts
)