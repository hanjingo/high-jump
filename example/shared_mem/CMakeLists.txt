cmake_minimum_required(VERSION 3.19.2)

project(shared_mem)

find_package(Boost)

if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
               main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    rt
    pthread)

else()
message("!!! boost not found, skip build example/shared_mem")

endif()