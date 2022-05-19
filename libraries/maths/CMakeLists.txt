cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(maths_library)
add_library(
	maths STATIC
	
	src/vsdebug.natvis

	src/vector.cpp
	src/matrix.cpp
	src/quaternion.cpp
	src/polynomial.cpp
	src/optimize.cpp
	src/assertion.cpp
)
target_include_directories(
	maths PUBLIC include
)