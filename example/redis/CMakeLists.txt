cmake_minimum_required(VERSION 3.19.2)

project(redis)

# find_package(hiredis QUIET)
find_package(hiredis REQUIRED)
if(NOT hiredis_FOUND)
    find_path(hiredis_INCLUDE_DIRS hiredis)
    find_library(hiredis_LIBRARIES hiredis)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${hiredis_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries (${PROJECT_NAME} ${hiredis_LIBRARIES})