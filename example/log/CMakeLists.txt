cmake_minimum_required(VERSION 3.19.2)

project(log)

find_package(spdlog)

if(spdlog_FOUND)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/3rd/spdlog/include)

add_executable(log 
               main.cpp)
target_link_libraries(${PROJECT_NAME} 
               pthread)

else()
message("!!! spdlog not found, skip build example/log")

endif()