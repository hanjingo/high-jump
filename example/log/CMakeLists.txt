cmake_minimum_required(VERSION 3.19.2)

project(log)

find_package(spdlog)

if(spdlog_FOUND)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(log 
               main.cpp)
target_link_libraries(${PROJECT_NAME} 
               pthread)
target_link_libraries(${PROJECT_NAME} 
               spdlog::spdlog)

else()
message("!!! spdlog not found, skip build example/log")

endif()