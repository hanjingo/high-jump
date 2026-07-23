#ifndef TESTING_HPP
#define TESTING_HPP

#ifdef HJ_ENABLE_BENCHMARK
#include <hj/testing/benchmark.hpp>
#endif

#ifdef HJ_ENABLE_CRASH
#include <hj/testing/crash.hpp>
#endif

#include <hj/testing/debugger.hpp>

#include <hj/testing/error_handler.hpp>

#include <hj/testing/error.hpp>

#include <hj/testing/exception.hpp>

#include <hj/testing/stacktrace.hpp>

#ifdef HJ_ENABLE_TELEMETRY
#include <hj/testing/telemetry.hpp>
#endif

#ifdef HJ_ENABLE_UNIT_TEST
#include <hj/testing/unit_test.hpp>
#endif

#endif // TESTING_HPP