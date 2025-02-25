cmake_minimum_required(VERSION 3.19.2)

project(ini)

find_package(Boost COMPONENTS 
    filesystem)

if(Boost_FOUND)
include_directories(${CMAKE_SOURCE_DIR})

include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

file(COPY cfg.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH})

else()
message("!!! boost not found, skip build example/ini")

endif()