cmake_minimum_required(VERSION 3.19.2)

project(task)

find_package(Boost COMPONENTS 
    filesystem
    thread)

if(Boost_FOUND)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(task 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

else()
message("!!! boost not found, skip build example/task")

endif()