#include <gtest/gtest.h>
#include <libcpp/testing/error.hpp>

enum err {
    err_ok = 0,
    err_fail,
    err_fatal,
};
using enum_err_v2_t = libcpp::error_v2<err>;
void on_err_fail(int& arg)
{
    arg = 1;
}
std::string on_err_fatal(int& arg)
{
    arg = 5;
    return std::string("on_err_fatal return");
}

TEST(error, error)
{
    ASSERT_EQ(ERROR(1) == ERROR(1), true);
    ASSERT_EQ(ERROR(1) != ERROR(1), false);
    ASSERT_EQ(ERROR("hello") == ERROR("hello"), true);
}

TEST(error, create)
{
    ASSERT_EQ(libcpp::error_factory::create(1).value(), 1);
}

TEST(error, code)
{
    ASSERT_EQ(libcpp::error_factory::create(1).value(), 1);
}

TEST(error, handle)
{
    int arg = 0;
    auto err_v2_fail = enum_err_v2_t(err_fail, (void*)(on_err_fail));
    err_v2_fail.handle(arg);
    ASSERT_EQ(arg, 1);

    auto err_v2_fatal = enum_err_v2_t(err_fatal, (void*)(on_err_fatal));
    auto ret = err_v2_fatal.handle<std::string>(arg);
    ASSERT_EQ(arg, 5);
    ASSERT_EQ(ret == std::string("on_err_fatal return"), true);
}