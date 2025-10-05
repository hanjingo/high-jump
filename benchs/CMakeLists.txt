
cmake_minimum_required(VERSION 3.19.2)

project(benchs)

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

find_package(benchmark REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME}
	benchmark::benchmark
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp
    OpenSSL::SSL 
    OpenSSL::Crypto
    spdlog::spdlog
)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} 
        ws2_32
    )
elseif(APPLE)
	target_link_libraries(${PROJECT_NAME} 
        pthread
    )
else()
	target_link_libraries(${PROJECT_NAME} 
        rt 
        pthread
    )
endif()

add_test(
    NAME ${PROJECT_NAME}
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> --benchmark_min_time=0.1
)

# Create results directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/results)

# Custom target to run benchmark with JSON output
string(TIMESTAMP BENCHMARK_TIMESTAMP "%Y%m%d_%H%M%S")
add_custom_target(benchmark_run
    COMMAND ${CMAKE_COMMAND} -E env 
        $<TARGET_FILE:${PROJECT_NAME}> 
        --benchmark_format=json 
        --benchmark_out=${CMAKE_CURRENT_SOURCE_DIR}/results/benchmark_${BENCHMARK_TIMESTAMP}.json
        --benchmark_min_time=0.1
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running benchmarks with JSON output"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Custom target to run benchmark with console output (default)
add_custom_target(benchmark_console
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> --benchmark_min_time=0.1
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running benchmarks with console output"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)