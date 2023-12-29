cmake_minimum_required(VERSION 3.19.2)

project(tcp_socket)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)

target_link_libraries(${PROJECT_NAME} 
    pthread)