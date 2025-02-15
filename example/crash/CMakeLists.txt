cmake_minimum_required(VERSION 3.19.2)

project(crash)

if (NOT BREAKPAD_INCLUDE_DIR)
    set(BREAKPAD_INCLUDE_DIR /usr/local/src/github.com/google/breakpad/src)
endif()

if (NOT BREAKPAD_LIB_DIR)
    set(BREAKPAD_LIB_DIR /usr/local/lib)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${BREAKPAD_INCLUDE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

link_directories(${BREAKPAD_LIB_DIR})
target_link_libraries(${PROJECT_NAME} ${BREAKPAD_LIB_DIR}/libbreakpad)