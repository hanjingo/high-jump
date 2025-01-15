cmake_minimum_required(VERSION 3.19.2)

project(libcpp)

# set environment param
set(LIBCPP_VERSION 0.0.1)

# set cmakes include path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakes)

# find_package(Boost)
# if(NOT Boost_FOUND)
#     set(BOOST_VERSION 1.80.0)
#     set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/3rd/boost)
#     include(FindFetchBoost)
# endif()

# find_package(ZeroMQ)
# if(NOT ZeroMQ_FOUND)
#     set(LIBZMQ_VERSION 4.3.4)
#     set(LIBZMQ_ROOT ${CMAKE_SOURCE_DIR}/3rd/zeromq)
#     include(FindFetchLibzmq)

#     set(ZMQPP_VERSION 4.2.0)
#     set(ZMQPP_ROOT ${CMAKE_SOURCE_DIR}/3rd/zmqpp)
#     include(FindFetchZmqpp)
# endif()

# find_package(spdlog)
# if(NOT spdlog_FOUND)
#     set(SPDLOG_FETCH_WAY git)
#     set(SPDLOG_VERSION 1.11.0)
#     set(SPDLOG_ROOT ${CMAKE_SOURCE_DIR}/3rd/spdlog)
#     include(FindFetchSpdlog)
# endif()

# find_package(GTest)
# if(NOT GTest_FOUND)
#     set(GTEST_FETCH_WAY https)
#     set(GTEST_VERSION 1.14.0)
#     set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/3rd/googletest)
#     include(FindFetchGTest)
# endif()

# find_package(Eigen3)
# if(NOT Eigen3_FOUND)
#     set(EIGEN3_FETCH_WAY git)
#     set(EIGEN3_VERSION 3.4)
#     set(EIGEN3_ROOT ${CMAKE_SOURCE_DIR}/3rd/eigen3)
#     include(FindFetchEigen3)
# endif()

# find_package(TBB)
# if(NOT TBB_FOUND)
#     set(TBB_FETCH_WAY https)
#     set(TBB_VERSION 2021.11.0)
#     set(TBB_ROOT ${CMAKE_SOURCE_DIR}/3rd/oneTBB)
#     include(FindFetchTBB)
# endif()

# build lib
if (BUILD_LIB)
    add_subdirectory(libcpp/binding)
endif()

# build example
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# build tests
if (BUILD_TEST)
    add_subdirectory(tests)
endif()