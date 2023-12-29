cmake_minimum_required(VERSION 3.19.2)

include_directories(${CMAKE_SOURCE_DIR})

project(safe_vector)

add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    pthread)