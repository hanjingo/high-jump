#include <exception>
#include <gtest/gtest.h>
#include <libcpp/testing/stacktrace.hpp>

// TEST(stacktrace, stacktrace)
// {
//     try
//     {
//         throw "stacktrace exception";
//     }
//     catch (...)
//     {
//         std::cout << libcpp::stacktrace() << std::endl;
//     }
// }

// TEST(stacktrace, recover)
// {
//     RECOVER(throw "throw exception";);
// }