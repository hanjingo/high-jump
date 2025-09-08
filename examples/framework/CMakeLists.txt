cmake_minimum_required(VERSION 3.19.2)

project(framework LANGUAGES CXX DESCRIPTION "The framework example of libcpp")

# Definitions
add_definitions(-DPACKAGE="${PROJECT_NAME}")
add_definitions(-DMAJOR_VERSION=0)
add_definitions(-DMINOR_VERSION=0)
add_definitions(-DPATCH_VERSION=1)
add_definitions(-DEMAIL="hehehunanchina@live.com")

# utf-8 support
if (MSVC)
    add_compile_options(/utf-8)
endif()

# options--code coverage
option(COVERAGE "Enable code coverage support" OFF)
# Options--library
option(BUILD_SHARED "Build shared object" OFF)
option(BUILD_STATIC "Build static object" OFF)
# Options--build
option(BUILD_TESTS "Build test" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)

# Environment require
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
elseif (CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "Please specify CMAKE_CXX_STANDARD of 11+")
endif()

# import vcpkg module
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

# Project Info
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/hanjingo/licpp/issues")

# Build Type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE 
        STRING "Build type config" FORCE)
endif()

# set C++ flags
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
endif()

# Avoid use of compiler language extensions, i.e. -std=c++11 not -std=gnu++11
if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

# windows specific settings
if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WIN32_WINNT=0x0A00     # Windows 10
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

# Output path
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()
if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# add test
if (BUILD_TESTS)
    add_subdirectory(test)
endif()

# add benchmark
if (BUILD_BENCHMARK)
    add_subdirectory(bench)
endif()

# add vcpkg library
find_package(opentelemetry-cpp CONFIG REQUIRED) # depend: spdlog, ...
find_package(ICU REQUIRED COMPONENTS uc i18n io)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(unofficial-breakpad REQUIRED)
find_package(Boost COMPONENTS 
    program_options
    filesystem)

# source files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
aux_source_directory(./src SRC)

# build exec file
add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} 
    ${CMAKE_DL_LIBS}
    ${Boost_LIBRARIES}
    ${ICU_LIBRARIES}
    ${OPENTELEMETRY_CPP_LIBRARIES}
    jwt-cpp::jwt-cpp
    fmt::fmt

    # breakpad_client
    unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
    unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
)