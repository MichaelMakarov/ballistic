add_library(url STATIC src/urlproc.cpp)
target_include_directories(url PUBLIC include)
target_link_libraries(url PRIVATE CURL::libcurl PRIVATE utility)
