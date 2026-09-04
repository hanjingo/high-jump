cmake_minimum_required(VERSION 3.19.2)

project(tests)

if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

add_definitions(-DSDK_BUILD)

# set binary output path
if (NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_library(sdk_v1 SHARED ${CMAKE_CURRENT_SOURCE_DIR}/sdk_v1.cpp)

target_link_libraries(sdk_v1 ${CMAKE_DL_LIBS})