cmake_minimum_required(VERSION 3.19.2)

project(sqlite)

find_package(SQLite3 REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries (${PROJECT_NAME} ${SQLite3_LIBRARIES})