#include <gtest/gtest.h>
#include <libcpp/testing/stacktrace.hpp>
#include <exception>

TEST(stacktrace, stacktrace)
{
    try {
        // int* n = new int(1);
        // delete n; n = nullptr;
        // int m = *n;
        throw "stacktrace exception";
    } catch (...) {
        // libcpp::stacktrace_t st;
        std::cout << libcpp::stacktrace() << std::endl;
    }
}