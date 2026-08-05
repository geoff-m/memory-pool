cmake_minimum_required(VERSION 3.28)
project(memory-pool)

set(CMAKE_CXX_STANDARD 20)
option(MEMORYPOOL_TEST_ENABLE
        "Whether memory-pool should build tests."
        ON
)

include(cmake/asan.cmake)
include(cmake/msan.cmake)
include(cmake/tsan.cmake)
include(cmake/ubsan.cmake)

add_library(memory-pool
        include/memory-pool/memory-pool.h
        src/memory-pool.cpp
        src/include/internal.h
        src/internal_linux.cpp
        src/internal_windows.cpp
        src/include/internal_linux.h
        src/include/internal_windows.h)

target_include_directories(memory-pool
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        PUBLIC $<INSTALL_INTERFACE:include/>
        PRIVATE src/include
)

# This propagates C++ standard 20 (at least) to targets that link to this.
target_compile_features(memory-pool INTERFACE cxx_std_20)

if (MEMORYPOOL_TEST_ENABLE)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(test)
endif()

include(GNUInstallDirs)
set_target_properties(memory-pool PROPERTIES PUBLIC_HEADER include/memory-pool/memory-pool.h)
install(TARGETS memory-pool
        EXPORT memory-pool-export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT memory-pool-export
        FILE memory-pool-export.cmake
        NAMESPACE memory-pool::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memory-pool
)

install(FILES cmake/memory-poolConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memory-pool
)