cmake_minimum_required(VERSION 3.17)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

set(BASIC_TEST_SOURCES basic_tests.cpp)
set(GRAPH_TEST_SOURCES graph_tests.cpp)
set(INLINING_TEST_SOURCES inlining_tests.cpp)
set(CHECK_ELIMINATION_TEST_SOURCES check_elimination_tests.cpp)

add_executable(basics_tests ${BASIC_TEST_SOURCES})
target_include_directories(basics_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/passes/include
)
target_link_libraries(basics_tests PRIVATE GTest::GTest compiler_lib passes_lib)
add_test(basics_tests_gtests basics_tests)

add_executable(graph_tests ${GRAPH_TEST_SOURCES})
target_include_directories(graph_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/passes/include
)
target_link_libraries(graph_tests PRIVATE GTest::GTest passes_lib compiler_lib)
add_test(graph_tests_gtests graph_tests)

add_executable(inlining_tests ${INLINING_TEST_SOURCES})
target_include_directories(inlining_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/passes/include
)
target_link_libraries(inlining_tests PRIVATE GTest::GTest compiler_lib passes_lib)
add_test(inlining_tests_gtests inlining_tests)

add_executable(check_elimination_tests ${CHECK_ELIMINATION_TEST_SOURCES})
target_include_directories(check_elimination_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/passes/include
)
target_link_libraries(check_elimination_tests PRIVATE GTest::GTest compiler_lib passes_lib)
add_test(check_elimination_tests_gtests check_elimination_tests)
