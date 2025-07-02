cmake_minimum_required(VERSION 3.19.2)

# import vcpkg module
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

project(libcpp)

# set environment param
set(LIBCPP_VERSION 1.0.0)

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
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
#    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# build lib
if (BUILD_LIB)
    add_subdirectory(libcpp/binding)
endif()

# build example
if(BUILD_EXAMPLE)
     add_subdirectory(example)
endif()

# build benmark
if (BUILD_TEST)
    add_subdirectory(tests)
    add_subdirectory(bench)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(libcpp INTERFACE)
target_include_directories(libcpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libcpp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmakes/libcppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libcppConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/libcpp
    # Passes the includedir install path
    PATH_VARS CMAKE_INSTALL_FULL_INCLUDEDIR
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libcppConfigVersion.cmake
    VERSION ${LIBCPP_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libcppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libcppConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_DATADIR}/libcpp
)

install(DIRECTORY libcpp/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcpp)

install(TARGETS libcpp 
        EXPORT libcppTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcpp
)

install(EXPORT libcppTargets 
    FILE libcppTargets.cmake
    DESTINATION ${CMAKE_INSTALL_DATADIR}/libcpp
)