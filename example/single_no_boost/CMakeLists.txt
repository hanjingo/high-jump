cmake_minimum_required(VERSION 3.19.2)

project(single_no_boost)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)