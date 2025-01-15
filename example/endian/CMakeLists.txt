cmake_minimum_required(VERSION 3.19.2)

project(endian)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)