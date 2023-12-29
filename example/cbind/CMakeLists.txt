cmake_minimum_required(VERSION 3.19.2)

project(cbind)

set(CMAKE_C_STANDARD 99)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libcpp/cbind)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_link_libraries(${PROJECT_NAME} libcpp-static)