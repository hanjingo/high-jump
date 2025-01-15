cmake_minimum_required(VERSION 3.19.2)

project(json)

include_directories(${CMAKE_SOURCE_DIR})

find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)