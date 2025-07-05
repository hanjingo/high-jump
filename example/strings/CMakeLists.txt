cmake_minimum_required(VERSION 3.19.2)

project(strings)

find_package(Boost)
find_package(fmt)

if(Boost_FOUND AND fmt_FOUND)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} fmt::fmt)

else()
message("!!! boost or fmt not found, skip build example/strings")

endif()