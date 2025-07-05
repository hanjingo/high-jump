cmake_minimum_required(VERSION 3.19.2)

project(coroutine)

find_package(Boost COMPONENTS
             context 
             thread)

if(Boost_FOUND)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

else()
message("!!! boost not found, skip build example/coroutine")

endif()