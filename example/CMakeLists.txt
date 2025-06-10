cmake_minimum_required(VERSION 3.19.2)

# cmake -DBUILD_EXAMPLE=ON ..

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# set binary output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
endif()

# set library output path
if (NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
endif()

# add_subdirectory(any)

# add_subdirectory(app)

add_subdirectory(base64)

# add_subdirectory(bits)

# # add_subdirectory(bt)

# add_subdirectory(bytes)

# # add_subdirectory(cbind)

# add_subdirectory(chan)

# add_subdirectory(coroutine)

# add_subdirectory(counter)

# add_subdirectory(cpu)

# add_subdirectory(crash)

# add_subdirectory(defer)

add_subdirectory(des)

# add_subdirectory(endian)

# add_subdirectory(env)

# add_subdirectory(exception)

# add_subdirectory(file)

# add_subdirectory(fork)

# add_subdirectory(hex)

# add_subdirectory(http)

# add_subdirectory(ini)

# add_subdirectory(init)

# add_subdirectory(ipc)

# add_subdirectory(json)

# add_subdirectory(log)

# add_subdirectory(matrix)

# add_subdirectory(md5)

# add_subdirectory(mux)

# add_subdirectory(once)

# add_subdirectory(optional)

# add_subdirectory(os)

# add_subdirectory(process)

# add_subdirectory(random)

# add_subdirectory(redis)

# add_subdirectory(reflect)

# add_subdirectory(ring_buf)

# add_subdirectory(sha256)

# add_subdirectory(shared_mem)

# add_subdirectory(signal)

# add_subdirectory(single)

# add_subdirectory(single_no_boost)

# add_subdirectory(sqlite)

# add_subdirectory(strings)

# add_subdirectory(striped_map)

# add_subdirectory(task)

# add_subdirectory(tcp)

# add_subdirectory(tcp_cs)

# add_subdirectory(tcp_socket)

# add_subdirectory(thread)

# add_subdirectory(time)

# add_subdirectory(timer)

# add_subdirectory(udp)

# # add_subdirectory(unit_test)

# add_subdirectory(uuid)

# add_subdirectory(variant)

# add_subdirectory(vector2d)

# add_subdirectory(xml)

# add_subdirectory(zmq)