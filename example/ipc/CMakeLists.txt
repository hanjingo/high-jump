cmake_minimum_required(VERSION 3.19.2)

project(ipc)

find_package(Boost)

if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
               main.cpp)

else()
message("!!! boost not found, skip build example/ipc")

endif()