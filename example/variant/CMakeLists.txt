cmake_minimum_required(VERSION 3.19.2)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(variant 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)