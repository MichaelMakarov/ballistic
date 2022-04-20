cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(maths_library)

add_subdirectory(maths)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	set(required_submodules google_test)
	option(gtest_force_shared_crt "" ON)

	update_submodules(${required_submodules} --init)
	
	enable_testing()
	add_subdirectory(tests)
endif()