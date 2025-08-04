cmake_minimum_required(VERSION 3.19.2)

project(child)

find_package(Boost REQUIRED COMPONENTS
    program_options
)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})