cmake_minimum_required(VERSION 3.17)

set(PASSES_SOURCES
    check_elimination.cpp
    inlining.cpp
    ir_builder.cpp
    loop_analyzer.cpp
    pass.cpp
)

add_library(passes_lib ${PASSES_SOURCES})

target_include_directories(passes_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(passes_lib compiler_lib)
