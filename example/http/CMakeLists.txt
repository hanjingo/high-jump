cmake_minimum_required(VERSION 3.19.2)

project(http)

find_package(httplib)

if(httplib_FOUND)
include_directories(${HTTPLIB_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
               main.cpp)
target_link_libraries(${PROJECT_NAME} pthread)

else()
message("!!! httplib not found, skip build example/http")

endif()