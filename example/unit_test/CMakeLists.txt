# cmake .. -DBUILD_EXAMPLE=ON
# make
# make test

cmake_minimum_required(VERSION 3.19.2)

project(unit_test)

enable_testing()

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${GTEST_INCLUDE_DIR})

add_executable(${PROJECT_NAME}  
    main.cpp
    hello_test.cpp 
    world_test.cpp)

target_link_libraries(${PROJECT_NAME} 
    ${GTEST_LIBRARIES} 
    pthread)

add_test(test ${PROJECT_NAME})