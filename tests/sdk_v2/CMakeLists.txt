cmake_minimum_required(VERSION 3.19.2)

project(tests)

if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

# set binary output path
if (NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# Symbol Visibility Control
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

include_directories(${CMAKE_SOURCE_DIR})

add_library(sdk_v2 SHARED ${CMAKE_CURRENT_SOURCE_DIR}/sdk_v2.cpp)

target_compile_definitions(
    sdk_v2
    PRIVATE
        SDK_BUILD
)

target_link_libraries(sdk_v2 ${CMAKE_DL_LIBS})