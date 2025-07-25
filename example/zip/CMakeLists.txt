# cmake .. -DBUILD_EXAMPLE=ON 
cmake_minimum_required(VERSION 3.19.2)

project(zip)

find_package(ZLIB CONFIG QUIET)
if(ZLIB_FOUND)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    ZLIB::ZLIB
)

else()
message("!!! zlib not found, skip build example/zip")

endif()