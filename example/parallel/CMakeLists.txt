cmake_minimum_required(VERSION 3.19.2)

project(parallel)

find_package(TBB REQUIRED) 
include_directories(${TBB_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(${PROJECT_NAME}
    TBB::tbb)   