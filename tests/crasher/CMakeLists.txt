cmake_minimum_required(VERSION 3.19.2)

project(crasher)

if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()

find_package(unofficial-breakpad REQUIRED)
find_package(Boost REQUIRED COMPONENTS
    program_options
)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}  
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES} # breakpad_client
    
    unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
    unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config
)