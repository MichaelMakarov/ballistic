add_library(
	ballistic STATIC
	src/jd.cpp
	src/gpt.cpp
	src/transform.cpp
	src/sunmoon.cpp	
)
target_include_directories(ballistic PUBLIC include)
target_link_libraries(ballistic PUBLIC mathlib parallel)