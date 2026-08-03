cmake_minimum_required(VERSION 3.28)
project(memory_pool)

set(CMAKE_CXX_STANDARD 20)
enable_testing()
include(FetchContent)
include(cmake/asan.cmake)
include(cmake/msan.cmake)
include(cmake/tsan.cmake)
include(cmake/ubsan.cmake)

add_library(memory_pool
        include/memory-pool/memory_pool.h
        src/memory_pool.cpp
        src/include/internal.h
        src/internal_linux.cpp
        src/internal_windows.cpp
        src/include/internal_linux.h
        src/include/internal_windows.h)

target_include_directories(memory_pool PUBLIC include)
target_include_directories(memory_pool PRIVATE src/include)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(test)