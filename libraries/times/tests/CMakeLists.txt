function(create_times_test filename)
	set(executable_name ${filename})
	add_executable(${executable_name} ${filename}.cpp)
	add_test(NAME ${executable_name} COMMAND $<TARGET_FILE:${executable_name}>)
	target_link_libraries(${executable_name} PRIVATE times gtest gtest_main)
endfunction()

create_times_test(datetime_test)
create_times_test(juliandate_test)