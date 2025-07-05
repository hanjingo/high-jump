cmake_minimum_required(VERSION 3.19.2)

find_package(libcpp REQUIRED)

# not recommand to use this, because it will include all headers in libcpp
# include_directories(${libcpp_INCLUDE_DIR})

add_executable(defer  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(defer PRIVATE libcpp)