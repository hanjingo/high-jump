cmake_minimum_required(VERSION 3.19.2)

project(test)

# utf-8 support
if (MSVC)
    add_compile_options(/utf-8)
endif()

# set binary output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# windows specific settings
if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WIN32_WINNT=0x0A00     # Windows 10
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

if(COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage -fprofile-arcs -ftest-coverage)
    
    # Find gcov
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(WARNING "gcov not found! Coverage reports will not be generated.")
    endif()
endif()

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} 
    ${SRC}
)

# For Github CI detection
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)

target_link_libraries(${PROJECT_NAME} 
    ${GTEST_LIBRARIES} 
)