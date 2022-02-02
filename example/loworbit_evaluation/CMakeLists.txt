add_executable(
	loworbit_evaluation
	src/loworbit_models.cpp
	src/dataprovider.cpp 
	src/auxilliares.cpp 
	src/model_wrapper.cpp
	src/main.cpp)
target_include_directories(loworbit_evaluation PRIVATE include)
target_link_libraries(loworbit_evaluation ballistic)
