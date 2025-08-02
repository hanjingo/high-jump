cmake_minimum_required(VERSION 3.19.2)

project(tests)

option(COVERAGE "Enable code coverage support" OFF)

# utf-8 support
if (MSVC)
    add_compile_options(/utf-8)
endif()

# set binary output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

# set windows version macro
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
endif()

enable_testing()

if(COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage -fprofile-arcs -ftest-coverage)
    
    # Find gcov
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(WARNING "gcov not found! Coverage reports will not be generated.")
    endif()
endif()

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
find_package(lz4 REQUIRED)
find_package(zstd REQUIRED)
find_package(ZLIB REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(cityhash REQUIRED)
find_package(unofficial-libharu REQUIRED)
find_package(Boost REQUIRED COMPONENTS
    system
    filesystem
    program_options
    context
    thread
)
find_package(OpenCL QUIET)
find_package(CUDA QUIET)

if (NOT WIN32)
    # for dpdk
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DPDK REQUIRED libdpdk)
endif()

if(OpenCL_FOUND)
    add_definitions(-DOPENCL_ENABLE)
    message(STATUS "OpenCL found, enabling OPENCL_ENABLE")
elseif(CUDA_FOUND)
    add_definitions(-DCUDA_ENABLE)
    message(STATUS "CUDA found, enabling CUDA_ENABLE")
else()
    message(WARNING "Neither OpenCL nor CUDA found, GPU features will be disabled.")
endif()

include_directories(${CMAKE_SOURCE_DIR})
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/person.pb.cc)

if (NOT WIN32)
    # for dpdk
    target_include_directories(${PROJECT_NAME} PRIVATE ${DPDK_INCLUDE_DIRS})
endif()

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
    ZLIB::ZLIB
    yaml-cpp::yaml-cpp
    nlohmann_json::nlohmann_json

#    breakpad_client
    unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
    unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config

#    concurrentqueue
    unofficial::concurrentqueue::concurrentqueue # for unofficial concurrentqueue cmake config

    unofficial::libharu::hpdf
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} 
        ws2_32 # for windows sockets
        bcrypt # for Windows cryptography

        hidapi::winapi
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        pthread # for *nix threads
        rt # for *nix real-time extensions

        hidapi::hidapi

#       clickhouse-cpp
        clickhouse-cpp-lib
        lz4::lz4
        zstd::libzstd
        cityhash

#       DPDK
        ${DPDK_LIBRARIES}
    )
endif()

if(OpenCL_FOUND)
    target_link_libraries(${PROJECT_NAME} 
        OpenCL::OpenCL
    )
elseif(CUDA_FOUND)
    target_link_libraries(${PROJECT_NAME} 
        ${CUDA_LIBRARIES}
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${CUDA_INCLUDE_DIRS})
endif()

add_test(test ${PROJECT_NAME})

file(COPY cfg.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY json_test.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY crypto.log DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY crypto_nopadding.log DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(dll_example)
add_subdirectory(child)
add_subdirectory(daemon)

add_subdirectory(shm_consumer)
add_subdirectory(shm_producer)