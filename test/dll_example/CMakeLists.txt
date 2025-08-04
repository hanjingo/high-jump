cmake_minimum_required(VERSION 3.19.2)

project(test)

# set binary output path
if (NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_library(dll_example SHARED ${CMAKE_CURRENT_SOURCE_DIR}/dll_example.cpp)

target_link_libraries(dll_example ${CMAKE_DL_LIBS})