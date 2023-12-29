cmake_minimum_required(VERSION 3.19.2)

project(zmq)

find_package(ZeroMQ REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${ZeroMQ_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/3rd/zmqpp/src)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(${PROJECT_NAME} pthread libzmq zmqpp)