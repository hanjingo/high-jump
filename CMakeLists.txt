cmake_minimum_required(VERSION 3.19.2)

# set environment param
project(hj 
    VERSION 1.0.0
    DESCRIPTION "Modern C++ Library for C++17 and above"
    LANGUAGES CXX
)

# set options
option(ASAN "Enable AddressSanitizer" OFF)
option(BUILD_TEST "Build the tests and benchmarks" OFF)
option(BUILD_LIB "Build the hj library" OFF)
option(BUILD_EXAMPLE "Build the example application" OFF)

# import vcpkg module
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

# set c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# for boost/stacktrace
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1")

# build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

# enable asan check
if (ASAN)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-volatile")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
        set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if (MSVC_VERSION GREATER_EQUAL 1928)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address /Zi")
            set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /fsanitize=address")
        else()
            message(WARNING "ASAN requires Visual Studio 2019 version 16.9 (MSVC 19.28) or newer. ASAN will be disabled.")
        endif()
    endif()
endif()

# build lib
if (BUILD_LIB)
    # DODO
endif()

# build example
if(BUILD_EXAMPLE)
     add_subdirectory(examples)
endif()

# build test
if (BUILD_TEST)
    enable_testing()

    add_subdirectory(tests)
endif()

# build benchmark
if (BUILD_BENCH)
    add_subdirectory(benchs)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(hj INTERFACE)
target_include_directories(hj INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hj>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmakes/hjConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/hjConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/hj
    # Passes the includedir install path
    PATH_VARS CMAKE_INSTALL_FULL_INCLUDEDIR
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/hjConfigVersion.cmake
    VERSION ${HJ_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/hjConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/hjConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_DATADIR}/hj
)

install(DIRECTORY hj/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hj)

install(TARGETS hj
        EXPORT hjTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hj
)

install(EXPORT hjTargets
    FILE hjTargets.cmake
    DESTINATION ${CMAKE_INSTALL_DATADIR}/hj
)