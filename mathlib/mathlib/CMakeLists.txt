if (MSVC)
    # to boost numeric computations
    add_compile_definitions(__AVX__)
endif()

add_library(
    mathlib STATIC
    src/maths.cpp 
    src/optimization.cpp 
    src/vsdebug.natvis
)
target_include_directories(mathlib PUBLIC include)
