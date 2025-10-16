cmake_minimum_required(VERSION 3.19.2)

project(shm_producer)

if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

find_package(Boost REQUIRED COMPONENTS
    program_options
)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES})

if(UNIX AND NOT APPLE) 
    target_link_libraries(${PROJECT_NAME} rt)
endif()