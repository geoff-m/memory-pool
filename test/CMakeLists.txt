cmake_minimum_required(VERSION 3.28)

add_executable(memory_pool_test
        src/TestSingleThread.cpp
        include/TestUtils.h
        src/TestUtils.cpp
)

target_include_directories(memory_pool_test PRIVATE include)

target_link_libraries(memory_pool_test
        PRIVATE memory_pool
        PRIVATE GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(memory_pool_test)