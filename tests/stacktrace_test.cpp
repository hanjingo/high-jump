
#include <exception>
#include <gtest/gtest.h>
#include <hj/testing/stacktrace.hpp>
#include <sstream>

TEST(stacktrace, stacktrace_basic)
{
    auto               st = hj::stacktrace();
    std::ostringstream oss;
    oss << st;
    EXPECT_GT(oss.str().size(), 0);
}

TEST(stacktrace, stacktrace_on_exception)
{
    std::ostringstream oss;
    auto               old_buf = std::cout.rdbuf(oss.rdbuf());
    try
    {
        throw "stacktrace exception";
    }
    catch(...)
    {
        std::cout << hj::stacktrace() << std::endl;
    }
    std::cout.rdbuf(old_buf);
    EXPECT_GT(oss.str().size(), 0);
}

TEST(stacktrace, recover_macro)
{
    std::ostringstream oss;
    auto               old_buf = std::cerr.rdbuf(oss.rdbuf());
    RECOVER(throw "throw exception";);
    std::cerr.rdbuf(old_buf);
    EXPECT_GT(oss.str().size(), 0);
}

TEST(stacktrace, stacktrace_multiple)
{
    std::ostringstream oss;
    auto               old_buf = std::cout.rdbuf(oss.rdbuf());
    for(int i = 0; i < 3; ++i)
    {
        try
        {
            throw "multi exception";
        }
        catch(...)
        {
            std::cout << hj::stacktrace() << std::endl;
        }
    }
    std::cout.rdbuf(old_buf);
    EXPECT_GT(oss.str().size(), 0);
}