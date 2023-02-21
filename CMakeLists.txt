cmake_minimum_required(VERSION 3.17)
project(compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror" CACHE STRING "Default CXX options" FORCE)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(passes)

set(COMPILER_SOURCES
    graph.cpp
    basic_block.cpp
    common.cpp
)

add_library(compiler_lib ${COMPILER_SOURCES})
target_include_directories(compiler_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/passes/include
)

add_executable(compiler compiler.cpp)
target_link_libraries(compiler compiler_lib passes_lib)

include(CTest)
enable_testing()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()