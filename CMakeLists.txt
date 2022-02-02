cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ballistic_library)

function (update_submodules required_submodules update_options)
	find_package(Git QUIET)
	if (GIT_FOUND)
		message(STATUS "intializing submodules")
		foreach (submodule ${required_submodules})
			set(submodule_path "${CMAKE_CURRENT_SOURCE_DIR}/external/${submodule}")
			if (NOT EXISTS "${submodule_path}/CMakeLists.txt")
				message(STATUS "updating ${submodule_path}")
				execute_process(COMMAND ${GIT_EXECUTABLE} submodule update ${update_options} -- ${submodule_path})
			endif()
		endforeach()
	endif()
endfunction()

set(required_submodules maths times)
update_submodules(${required_submodules} --init)
foreach (submodule ${required_submodules})
	add_subdirectory("external/${submodule}")
endforeach()

add_subdirectory(ballistic)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	set(required_submodules wjson)
	update_submodules(${required_submodules} --init --recursive)

	add_subdirectory(example)
	
endif()
