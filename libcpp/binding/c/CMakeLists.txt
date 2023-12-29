cmake_minimum_required(VERSION 3.19.2)

project(libcpp)

find_package(pugixml REQUIRED)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DLIBCPP_EXPORT)

if (NOT SHARED_NAME)
    set(SHARED_NAME libcpp)
endif()

if (NOT STATIC_NAME)
    set(STATIC_NAME libcpp-static)
endif()

# set library output path
if (NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${PUGIXML_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libcpp/binding/c)

aux_source_directory(./src SRC)

add_library(${SHARED_NAME} SHARED ${SRC})
add_library(${STATIC_NAME} STATIC ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)