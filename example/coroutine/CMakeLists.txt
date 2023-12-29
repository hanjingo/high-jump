cmake_minimum_required(VERSION 3.19.2)

project(coroutine)

find_package(Boost REQUIRED
             context 
             thread)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})