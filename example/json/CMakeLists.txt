cmake_minimum_required(VERSION 3.19.2)

project(json)

find_package(nlohmann_json)

if(nlohmann_json_FOUND)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

else()
message("!!! nlohmann_json not found, skip build example/json")

endif()