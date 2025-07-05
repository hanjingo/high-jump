cmake_minimum_required(VERSION 3.19.2)

project(chan)

find_package(concurrentqueue REQUIRED)
include_directories(/usr/local/include)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    pthread)