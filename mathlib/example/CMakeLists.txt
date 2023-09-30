add_executable(
    mathexample 
    src/main.cpp
    src/testmat.cpp
    src/testvec.cpp
    src/testmatrix.cpp
    src/testvector.cpp
    src/testpoly.cpp
    src/testquaternion.cpp
    src/auxiliaries.cpp
)

target_include_directories(mathexample PRIVATE include)
target_link_libraries(mathexample PRIVATE mathlib)
