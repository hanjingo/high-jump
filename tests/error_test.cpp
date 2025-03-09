#include <gtest/gtest.h>
#include <libcpp/testing/error.hpp>

enum class err1
{
    ok = 0,
    fail,
    unknown,
};

TEST(error, err_to_int)
{
    ASSERT_EQ(libcpp::err_to_int(err1::ok) == 0, true);
}

TEST(error, err_to_hex)
{
    ASSERT_EQ(libcpp::err_to_hex(err1::ok) == "0x0", true);

    unsigned long long err2 = 1024;
    ASSERT_EQ(libcpp::err_to_hex(err2) == "0x400", true);
}