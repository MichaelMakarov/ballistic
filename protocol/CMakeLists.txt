add_library(protocol STATIC src/protocol.cpp)
target_include_directories(protocol PUBLIC include)
target_link_libraries(protocol PRIVATE libutility)
