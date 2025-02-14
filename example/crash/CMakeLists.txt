cmake_minimum_required(VERSION 3.19.2)

project(crash)

set (BREAKPAD_LIB_DIR /usr/local/lib)

include_directories(${CMAKE_SOURCE_DIR}/3rd/breakpad/src)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

link_directories(${BREAKPAD_LIB_DIR})
target_link_libraries(${PROJECT_NAME} breakpad)