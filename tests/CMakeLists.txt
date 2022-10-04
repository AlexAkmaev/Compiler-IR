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

set(BASIC_TEST_SOURCES basic_tests.cpp ${PROJECT_SOURCE_DIR}/Graph.cpp ${PROJECT_SOURCE_DIR}/BasicBlock.cpp)

add_executable(basics_tests ${BASIC_TEST_SOURCES})
target_include_directories(basics_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(basics_tests PRIVATE GTest::GTest)
add_test(basics_tests_gtests basics_tests)
