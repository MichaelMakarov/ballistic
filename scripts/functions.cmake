# updates a submodule using specified options
function (update_submodule submodule options)
	set (submodule_path "${CMAKE_CURRENT_SOURCE_DIR}/${submodule}")
	if (NOT EXISTS "${submodule_path}/CMakeLists.txt")
		find_package(Git QUIET)
		if (GIT_FOUND)
			message(STATUS "Updating ${submodule_path}")
			execute_process(COMMAND ${GIT_EXECUTABLE} submodule update ${update_options} -- ${submodule_path} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		else()
			message(STATUS "Failed to update submodule ${submodule_path}. Do it manually.")
		endif()
	endif()
endfunction()
