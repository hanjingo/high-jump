cmake_minimum_required(VERSION 3.19.2)

# Load Environment Info
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/env.cmake)
load_env("${CMAKE_CURRENT_SOURCE_DIR}/.env")

# set project
project(${PACKAGE_NAME} 
    LANGUAGES CXX 
    DESCRIPTION ${PACKAGE_DESCRIPTION_SUMMARY}
)

# options--code check
option(ASAN "Enable AddressSanitizer" OFF)
option(COVERAGE "Enable code coverage support" OFF)
# Options--library
option(BUILD_SHARED "Build shared object" ON)
option(BUILD_STATIC "Build static object" OFF)
# Options--build
option(BUILD_TEST "Build test" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)

# import vcpkg module
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

# utf-8 support
if (MSVC)
    add_compile_options(/utf-8)
endif()

# Environment require
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
elseif (CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Please specify CMAKE_CXX_STANDARD of 17+")
endif()

# set c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type config" FORCE)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

# set output path
if (NOT LIBRARY_OUTPUT_PATH )
    set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/bin)
endif()

# Avoid use of compiler language extensions, i.e. -std=c++11 not -std=gnu++11
if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

# set C++ base flags
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
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

# Macro Definitions
add_definitions(-DPACKAGE="${PACKAGE_NAME}")
add_definitions(-DMAJOR_VERSION=${PACKAGE_VERSION_MAJOR})
add_definitions(-DMINOR_VERSION=${PACKAGE_VERSION_MINOR})
add_definitions(-DPATCH_VERSION=${PACKAGE_VERSION_PATCH})
add_definitions(-DEMAIL="${PACKAGE_CONTACT}")

# windows specific settings
if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WIN32_WINNT=0x0A00     # Windows 10
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

# add test
if (BUILD_TEST)
    add_subdirectory(test)
endif()

# add benchmark
if (BUILD_BENCHMARK)
    add_subdirectory(bench)
endif()

# add vcpkg library
find_package(fmt REQUIRED)
find_package(unofficial-breakpad REQUIRED)

# source files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
aux_source_directory(./src SRC)

# build lib file
if (BUILD_SHARED)
    set(SDK_SHARED_NAME "${PROJECT_NAME}")
    add_library(${SDK_SHARED_NAME} SHARED ${SRC})

    # link library
    if (WIN32)
        target_link_libraries(${SDK_SHARED_NAME} 
            ${CMAKE_DL_LIBS}
            bcrypt # for Windows cryptography

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    elseif(APPLE)
        target_link_libraries(${SDK_SHARED_NAME} 
            ${CMAKE_DL_LIBS}
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    else() # Linux
        target_link_libraries(${SDK_SHARED_NAME} 
            ${CMAKE_DL_LIBS}
            rt
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    endif()

    # Deploy dependencies
    # for windows + vs2019:
    # cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -G "Visual Studio 16 2019" -DBUILD_SHARED=ON
    # for *nix:
    # cmake .. -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" -DBUILD_SHARED=ON
    #
    # cmake --build .
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy.cmake)
    if (WIN32)
        win_deploy(${SDK_SHARED_NAME}
            ${CMAKE_DL_LIBS}
            bcrypt

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    elseif(APPLE)
        mac_deploy(${SDK_SHARED_NAME}
            ${CMAKE_DL_LIBS}
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    else()
        linux_deploy(${SDK_SHARED_NAME}
            ${CMAKE_DL_LIBS}
            rt
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    endif()
endif()

if (BUILD_STATIC)
    set(SDK_STATIC_NAME "${PROJECT_NAME}_static")
    add_library(${SDK_STATIC_NAME} STATIC ${SRC})

    # for os-specific libraries
    if (WIN32)
        target_link_libraries(${SDK_STATIC_NAME} 
            ${CMAKE_DL_LIBS}
            bcrypt # for Windows cryptography

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    elseif(APPLE)
        target_link_libraries(${SDK_STATIC_NAME} 
            ${CMAKE_DL_LIBS}
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    else() # Linux
        target_link_libraries(${SDK_STATIC_NAME} 
            ${CMAKE_DL_LIBS}
            rt
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
            unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
        )
    endif()

    # Deploy dependencies
    # for windows + vs2019:
    # cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -G "Visual Studio 16 2019" -DBUILD_STATIC=ON
    # for *nix:
    # cmake .. -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" -DBUILD_STATIC=ON
    #
    # cmake --build .
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy.cmake)
    if (WIN32)
        win_deploy(${SDK_STATIC_NAME}
            ${CMAKE_DL_LIBS}
            bcrypt

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    elseif(APPLE)
        mac_deploy(${SDK_STATIC_NAME}
            ${CMAKE_DL_LIBS}
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    else()
        linux_deploy(${SDK_STATIC_NAME}
            ${CMAKE_DL_LIBS}
            rt
            pthread

            fmt::fmt
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
        )
    endif()
endif()

