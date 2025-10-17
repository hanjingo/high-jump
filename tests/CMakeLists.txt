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

# windows specific settings
if (WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WIN32_WINNT=0x0A00     # Windows 10
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )

    add_compile_options(/bigobj)
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

find_package(PkgConfig QUIET)
find_package(benchmark REQUIRED)
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
find_package(jwt-cpp CONFIG REQUIRED)
find_package(hidapi REQUIRED)
find_package(unofficial-breakpad REQUIRED)
find_package(unofficial-concurrentqueue REQUIRED)
find_package(lz4 REQUIRED)
find_package(zstd REQUIRED)
find_package(ZLIB REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(cityhash REQUIRED)
find_package(unofficial-libharu REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n io)
find_package(opentelemetry-cpp CONFIG REQUIRED) # depend: spdlog, ...
find_package(clickhouse-cpp QUIET) # depend: lz4, zstd, cityhash
find_package(libpqxx REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(gRPC CONFIG QUIET)
find_package(hffix REQUIRED)
find_package(Boost REQUIRED COMPONENTS
    system
    filesystem
    program_options
    context
    thread
)
find_package(OpenCL QUIET)
find_package(CUDA QUIET)

# for qrcode encode
find_path(QRENCODE_INCLUDE_DIR NAMES qrencode.h)
find_library(QRENCODE_LIBRARY_RELEASE qrencode)
find_library(QRENCODE_LIBRARY_DEBUG qrencoded)
set(QRENCODE_LIBRARIES optimized ${QRENCODE_LIBRARY_RELEASE} debug ${QRENCODE_LIBRARY_DEBUG})
# for qrcode decode
find_package(quirc CONFIG REQUIRED)

# DPDK support - optional and only on non-Windows platforms
if(NOT WIN32)
    if(PkgConfig_FOUND)
        pkg_check_modules(DPDK QUIET libdpdk)
        if(DPDK_FOUND)
            add_definitions(-DDPDK_ENABLE)
            message(STATUS "DPDK found, enabling DPDK")
        else()
            message(WARNING "DPDK requested but not found")
        endif()
    else()
        message(WARNING "PkgConfig not found, cannot search for DPDK")
    endif()
elseif(ENABLE_DPDK AND WIN32)
    message(WARNING "DPDK is not supported on Windows, disabling DPDK support")
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
if(gRPC_FOUND)
    add_definitions(-DGRPC_ENABLE)
    message(STATUS "gRPC found, enabling gRPC support")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/grpc)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/grpc/grpc_test.pb.cc)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/grpc/grpc_test.grpc.pb.cc)
endif()
add_executable(${PROJECT_NAME} ${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/person.pb.cc)

# For Github CI detection
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)

# Add DPDK include directories if found
if(DPDK_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${DPDK_INCLUDE_DIRS})
endif()

# Add libqrencode include directories and link libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${QRENCODE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES} 
    ${GTEST_LIBRARIES} 
    ${SQLite3_LIBRARIES} 
    ${Eigen_LIBRARIES} 
    ${ODBC_LIBRARIES}
    ${ICU_LIBRARIES}
    ${OPENTELEMETRY_CPP_LIBRARIES}
    ${QRENCODE_LIBRARIES}
    quirc::quirc
    libzmq
    benchmark::benchmark
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
    jwt-cpp::jwt-cpp
    libpqxx::pqxx
    hiredis::hiredis

    # breakpad_client
    unofficial::breakpad::libbreakpad # for unofficial breakpad cmake config
    unofficial::breakpad::libbreakpad_client # for unofficial breakpad-client cmake config

    # concurrentqueue
    unofficial::concurrentqueue::concurrentqueue # for unofficial concurrentqueue cmake config

    # pdf
    unofficial::libharu::hpdf
)

# for os-specific libraries
if (WIN32)
    target_link_libraries(${PROJECT_NAME} 
        ws2_32 # for windows sockets
        bcrypt # for Windows cryptography
        hidapi::winapi
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        pthread
        hidapi::hidapi

        "-framework CoreGraphics" # for keyboard,mouse api
    )
else() # Linux
    target_link_libraries(${PROJECT_NAME} 
        rt
        pthread
        hidapi::hidapi
    )
endif()

# Link grpc if found
if(gRPC_FOUND)
    message(STATUS "gRPC found, enabling gRPC support link")
    target_link_libraries(${PROJECT_NAME} 
        # grpc
        gRPC::grpc
        gRPC::grpc++ 
    )

    if (WIN32)
        target_link_libraries(${PROJECT_NAME} 
            gRPC::grpc++_reflection # for windows grpc
        )
    endif()
endif()

# Link DPDK libraries if found
if(DPDK_FOUND)
    target_link_libraries(${PROJECT_NAME} 
        ${DPDK_LIBRARIES}
    )
    message(STATUS "Linking DPDK libraries: ${DPDK_LIBRARIES}")
endif()

# Link clickhouse if found
if(TARGET clickhouse-cpp-lib)
    add_definitions(-DCLICKHOUSE_ENABLE)
    message(STATUS "clickhouse-cpp found, enabling ClickHouse support")
    target_link_libraries(${PROJECT_NAME} 
        clickhouse-cpp-lib 
        lz4::lz4 
        zstd::libzstd 
        cityhash
    )
endif()

# opencl or cuda
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

# test config files copy
file(GLOB TEST_CONFIG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cfg.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/json_test.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/crypto.log"
    "${CMAKE_CURRENT_SOURCE_DIR}/crypto_nopadding.log"
    "${CMAKE_CURRENT_SOURCE_DIR}/client.crt"
    "${CMAKE_CURRENT_SOURCE_DIR}/client.key"
    "${CMAKE_CURRENT_SOURCE_DIR}/server.crt"
    "${CMAKE_CURRENT_SOURCE_DIR}/server.key"
)
add_custom_target(copy_test_configs ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TEST_CONFIG_FILES}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    DEPENDS ${TEST_CONFIG_FILES}
    COMMENT "Copying test config files to output dir"
)
add_dependencies(${PROJECT_NAME} copy_test_configs)

add_subdirectory(dll_example)
add_subdirectory(child)
add_subdirectory(daemon)

add_subdirectory(shm_consumer)
add_subdirectory(shm_producer)