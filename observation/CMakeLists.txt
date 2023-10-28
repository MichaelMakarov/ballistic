add_library(
    observation STATIC 
    src/observation_json.cpp 
    src/observation_tle.cpp 
    src/SGP4.cpp
)
target_include_directories(observation PUBLIC include)
target_link_libraries(
    observation PUBLIC 
    times 
    json 
    utility 
    mathlib 
    ballistic
)