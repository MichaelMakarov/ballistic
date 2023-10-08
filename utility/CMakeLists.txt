add_library(
    utility STATIC 
    src/fileutils.cpp 
    src/printutils.cpp
    src/times.cpp
)
target_include_directories(utility PUBLIC include)