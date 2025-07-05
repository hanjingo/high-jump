cmake_minimum_required(VERSION 3.19.2)

project(shared_mem)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
               main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    rt
    pthread)