cmake_minimum_required(VERSION 3.19.2)

project(redis)

find_package(hiredis QUIET)
if(NOT hiredis_FOUND)
    find_path(HIREDIS_INCLUDE_DIRS hiredis)
    find_library(HIREDIS_LIB hiredis)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${HIREDIS_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries (${PROJECT_NAME} ${HIREDIS_LIB})