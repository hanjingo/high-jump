cmake_minimum_required(VERSION 3.19.2)

project(ini)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)