cmake_minimum_required(VERSION 3.19.2)

project(log)

find_package(spdlog REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/3rd/spdlog/include)

add_executable(log 
               main.cpp)
target_link_libraries(${PROJECT_NAME} 
               pthread)