cmake_minimum_required(VERSION 3.19.2)

project(shm_producer)

find_package(Boost COMPONENTS 
    program_options)

if(Boost_FOUND)
    include_directories(${CMAKE_SOURCE_DIR})

    add_executable(${PROJECT_NAME}  
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )

    target_link_libraries(${PROJECT_NAME} 
        ${Boost_LIBRARIES}
        rt)
else()
    message("!!! boost not found, skip build tests/shm_producer")
    
endif()