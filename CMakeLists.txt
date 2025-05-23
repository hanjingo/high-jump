cmake_minimum_required(VERSION 3.19.2)

project(libcpp)

# set environment param
set(LIBCPP_VERSION 1.0.0)

# for boost/stacktrace
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1")

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
    add_subdirectory(bench)
endif()

add_subdirectory(tests)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/libcpp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)