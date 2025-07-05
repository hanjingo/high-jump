cmake_minimum_required(VERSION 3.19.2)

project(xml)

find_package(pugixml REQUIRED)
include_directories(${PUGIXML_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)