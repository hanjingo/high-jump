# cmake .. -DBUILD_EXAMPLE=ON 
cmake_minimum_required(VERSION 3.19.2)

project(pdf)

find_package(unofficial-libharu CONFIG QUIET)
if(unofficial-libharu_FOUND)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    unofficial::libharu::hpdf
)

else()
message("!!! libharu not found, skip build example/pdf")

endif()