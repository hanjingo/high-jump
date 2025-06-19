cmake_minimum_required(VERSION 3.19.2)

project(tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set binary output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# set windows version macro
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
endif()

enable_testing()

find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(httplib REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(pugixml REQUIRED)
find_package(Protobuf REQUIRED)
find_package(TBB REQUIRED)
find_package(ODBC REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(hidapi REQUIRED)
find_package(unofficial-breakpad REQUIRED)
find_package(unofficial-concurrentqueue REQUIRED)
find_package(Boost REQUIRED COMPONENTS
    system
    filesystem
    program_options
    context
    thread
)

include_directories(${CMAKE_SOURCE_DIR})
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/person.pb.cc)

target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES} 
    ${GTEST_LIBRARIES} 
    ${SQLite3_LIBRARIES} 
    ${Eigen_LIBRARIES} 
    ${ODBC_LIBRARIES}
    protobuf::libprotobuf
    pugixml
    BT::behaviortree_cpp
    OpenSSL::SSL 
    OpenSSL::Crypto
    pugixml
    fmt::fmt
    TBB::tbb
    nlohmann_json::nlohmann_json
    hidapi::winapi

#    breakpad_client
    unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
    unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config

#    concurrentqueue
    unofficial::concurrentqueue::concurrentqueue # for unofficial concurrentqueue cmake config
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} 
        ws2_32 # for windows sockets
        bcrypt # for Windows cryptography
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        pthread # for *nix threads
        rt # for *nix real-time extensions
    )
endif()

add_test(test ${PROJECT_NAME})

file(COPY cfg.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(dll_example)
add_subdirectory(child)
add_subdirectory(daemon)

add_subdirectory(shm_consumer)
add_subdirectory(shm_producer)