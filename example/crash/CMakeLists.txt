cmake_minimum_required(VERSION 3.19.2)

project(crash)

include_directories(${CMAKE_SOURCE_DIR})

#if (NOT Breakpad_INCLUDE_DIR)
#    set(Breakpad_INCLUDE_DIR /usr/local/include/breakpad)
#endif()
#include_directories(${Breakpad_INCLUDE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

#if (NOT Breakpad_LIB_DIR)
#    set(Breakpad_LIB_DIR /usr/local/lib)
#endif()
#link_directories(${Breakpad_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
    breakpad_client
    pthread)