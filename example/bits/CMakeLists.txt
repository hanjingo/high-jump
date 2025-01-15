cmake_minimum_required(VERSION 3.19.2)

project(bits)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)