add_library(
    graphic STATIC 
    src/graphic_window.cpp
    src/graphic_drawer.cpp
)
target_include_directories(graphic PUBLIC include)
target_link_libraries(graphic PRIVATE Qt5::Widgets Qt5::Charts)