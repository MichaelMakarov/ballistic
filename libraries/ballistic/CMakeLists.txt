add_library(
	ballistic 
	STATIC
	src/atmosphere1981.cpp 
	src/atmosphere2004.cpp
	src/egm.cpp
	src/solar_system.cpp
	src/geopotential.cpp
	src/transform.cpp
)
target_include_directories(ballistic PUBLIC include)
target_link_libraries(ballistic PUBLIC maths times)