cmake_minimum_required(VERSION 3.19.2)

project(endian)

find_package(Boost)

if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)

else()
message("!!! boost not found, skip build example/endian")

endif()