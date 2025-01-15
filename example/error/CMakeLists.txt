cmake_minimum_required(VERSION 3.19.2)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(error 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)