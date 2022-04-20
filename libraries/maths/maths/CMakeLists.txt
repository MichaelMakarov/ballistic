add_library(
	maths
	STATIC
	src/vector.cpp
	src/matrix.cpp
	src/vsdebug.natvis
	src/functions.cpp
	src/quaternion.cpp
	src/polynomial.cpp
	src/assertion.cpp)
target_include_directories(maths PUBLIC include)