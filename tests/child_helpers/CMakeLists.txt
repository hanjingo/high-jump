cmake_minimum_required(VERSION 3.19.2)

project(child_helpers)

if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

include_directories(${Boost_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)