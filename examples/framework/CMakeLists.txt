cmake_minimum_required(VERSION 3.19.2)

# Load Environment Info
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/env.cmake)
load_env("${CMAKE_CURRENT_SOURCE_DIR}/.env")

# set project
project(${PACKAGE_NAME} 
    LANGUAGES C CXX 
    DESCRIPTION ${PACKAGE_DESCRIPTION_SUMMARY}
)

# options--code check
option(ASAN "Enable AddressSanitizer" OFF)
option(COVERAGE "Enable code coverage support" OFF)
# Options--library
option(BUILD_SHARED "Build shared object" OFF)
option(BUILD_STATIC "Build static object" OFF)
# Options--build
option(BUILD_TEST "Build test" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)

# Load Environment Info
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/env.cmake)
load_env("${CMAKE_CURRENT_SOURCE_DIR}/.env")

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
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
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
add_definitions(-DLIC_ENABLE=${LIC_ENABLE})
add_definitions(-DLIC_ISSUER="${LIC_ISSUER}")
add_definitions(-DLIC_FPATH="${LIC_FPATH}")
add_definitions(-DCRASH_HANDLER_ENABLE=${CRASH_HANDLER_ENABLE})
add_definitions(-DTELEMETRY_ENABLE=${TELEMETRY_ENABLE})
add_definitions(-DPROPERTY_ENABLE=${PROPERTY_ENABLE})

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
find_package(hj REQUIRED)

# source files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
aux_source_directory(./src SRC)

# build exec file
add_executable(${PROJECT_NAME} ${SRC})

# link library
set(LINK_LIBS ${CMAKE_DL_LIBS} ${hj_LIBRARIES})
if(WIN32)
    list(APPEND LINK_LIBS 
        bcrypt
    )
elseif(APPLE)
    list(APPEND LINK_LIBS 
        pthread
    )
else()
    list(APPEND LINK_LIBS 
        rt 
        pthread
    )
endif()
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

file(GLOB RESOURCE_FILES CONFIGURE_DEPENDS "")
if (PROPERTY_ENABLE)
    # copy properties files
    file(GLOB PROPERTY_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/res/*.properties"
    )
    list(APPEND RESOURCE_FILES ${PROPERTY_FILES})
endif()
if (LIC_ENABLE)
    # copy license file
    file(GLOB LIC_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/res/*.lic"
    )
    list(APPEND RESOURCE_FILES ${LIC_FILES})
endif()

if (RESOURCE_FILES)
    add_custom_target(${PACKAGE_NAME}_copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RESOURCE_FILES}
            "${EXECUTABLE_OUTPUT_PATH}"
        DEPENDS ${RESOURCE_FILES}
        COMMENT "Copying resource files to output dir"
    )
else()
    message(STATUS "No resource files to copy for ${PROJECT_NAME}")
endif()

# Crash Trace
if (CRASH_HANDLER_ENABLE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/triplet.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/crash.cmake)
    generate_sym(${PACKAGE_NAME} "${EXECUTABLE_OUTPUT_PATH}/symbols/${PACKAGE_NAME}")
endif()

# Deploy dependencies
# for windows + vs2019:
# cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -G "Visual Studio 16 2019"
# for *nix:
# cmake .. -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
#
# cmake --build .
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy.cmake)
if(WIN32)
    win_deploy(${PROJECT_NAME} 
        ${LINK_LIBS}
    )
elseif(APPLE)
    mac_deploy(${PROJECT_NAME} 
        ${LINK_LIBS}
    )
else()
    linux_deploy(${PROJECT_NAME} 
        ${LINK_LIBS}
    )
endif()

# Package
# for windows + NSIS:
# cpack -G NSIS -C Debug
#
# for linux + DEB:
# cpack -G DEB -C Debug
#
# for mac + dmg:
# cpack -G DragNDrop -C Debug
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pack.cmake)