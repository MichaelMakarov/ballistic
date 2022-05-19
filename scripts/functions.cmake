function (update_submodules required_submodules update_options)
	find_package(Git QUIET)
	if (GIT_FOUND)
		message(STATUS "intializing submodules")
		foreach (submodule ${required_submodules})
			set(submodule_path "${CMAKE_SOURCE_DIR}/external/${submodule}")
			if (NOT EXISTS "${submodule_path}/CMakeLists.txt")
				message(STATUS "updating ${submodule_path}")
				execute_process(COMMAND ${GIT_EXECUTABLE} submodule update ${update_options} -- ${submodule_path})
			endif()
		endforeach()
	endif()
endfunction()

function(add_external_project projname srcdir)
	ExternalProject_Add(
		${projname}
		SOURCE_DIR ${srcdir}
		PREFIX ${projname}
		INSTALL_COMMAND ""
	)
endfunction()
