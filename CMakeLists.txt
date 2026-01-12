cmake_minimum_required(VERSION 3.28)
project(memory_pool)

set(CMAKE_CXX_STANDARD 20)
enable_testing()
Set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

add_library(memory_pool
        include/memory_pool.h
        src/memory_pool.cpp
        src/include/internal.h
        src/internal_linux.cpp)

target_include_directories(memory_pool PUBLIC include)
target_include_directories(memory_pool PRIVATE src/include)

add_subdirectory(test)

find_package(GoogleTest QUIET)
if(GoogleTest_FOUND)
    message(STATUS "Will use found GoogleTest package")
else()
    message(STATUS "GoogleTest package not found--will attempt to download it")
    FetchContent_Declare(
            GoogleTest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # v1.17.0
    )
    FetchContent_MakeAvailable(GoogleTest)
endif()
