cmake_minimum_required(VERSION 3.19.2)

project(libcpp)

# set environment param
set(LIBCPP_VERSION 0.0.1)

# build lib
if (BUILD_LIB)
    add_subdirectory(libcpp/binding)
endif()

# build example
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# build tests
if (BUILD_TEST)
    add_subdirectory(tests)
endif()