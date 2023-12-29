# cmake .. -DBUILD_EXAMPLE=ON -DUSE_BOOST=ON

cmake_minimum_required(VERSION 3.19.2)

project(bt)

find_package(Boost REQUIRED 
    coroutine)
find_package(behaviortree_cpp REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
               main.cpp)
target_link_libraries(${PROJECT_NAME} 
    BT::behaviortree_cpp
    ${Boost_LIBRARIES})
