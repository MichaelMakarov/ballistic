add_library(
	times 
	STATIC
	src/datetime.cpp
	src/juliandate.cpp
	src/stopwatch.cpp)
target_include_directories(times PUBLIC include)