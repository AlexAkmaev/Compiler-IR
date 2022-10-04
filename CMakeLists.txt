cmake_minimum_required(VERSION 3.17)
project(compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror" CACHE STRING "Default CXX options" FORCE)
set(CMAKE_CXX_STANDARD 17)

set(COMPILER_SOURCES
        compiler.cpp
    Graph.cpp
        BasicBlock.cpp)

add_executable(compiler ${COMPILER_SOURCES})
target_include_directories(compiler
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(CTest)
enable_testing()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()