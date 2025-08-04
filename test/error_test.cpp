#include <gtest/gtest.h>
#include <libcpp/testing/error.hpp>

enum class err1
{
    ok = -1,
    fail,
    unknown,
};

TEST(error, err_to_int)
{
    ASSERT_EQ(libcpp::err_to_int(err1::ok) == -1, true);
    ASSERT_EQ(libcpp::err_to_int(err1::fail) == 0, true);
    ASSERT_EQ(libcpp::err_to_int(err1::unknown) == 1, true);

    ASSERT_EQ(libcpp::err_to_int(err1::ok, 1) == 1, true);
    ASSERT_EQ(libcpp::err_to_int(err1::fail, 1) == 0, true);
    ASSERT_EQ(libcpp::err_to_int(err1::unknown, 1) == 1, true);
}

TEST(error, err_to_hex)
{
    ASSERT_EQ(libcpp::err_to_hex(err1::ok) == "0xFFFFFFFF", true);
    ASSERT_EQ(libcpp::err_to_hex(err1::ok, false) == "0xffffffff", true);
    ASSERT_EQ(libcpp::err_to_hex(err1::ok, false, "err-") == "err-ffffffff",
              true);
    ASSERT_EQ(libcpp::err_to_hex(err1::fail) == "0x0", true);
    ASSERT_EQ(libcpp::err_to_hex(err1::unknown) == "0x1", true);

    std::uint16_t err2 = 1024;
    ASSERT_EQ(libcpp::err_to_hex(err2) == "0x400", true);

    std::uint16_t err3 = 65535;
    ASSERT_EQ(libcpp::err_to_hex(err3) == "0xFFFF", true);
    ASSERT_EQ(libcpp::err_to_hex(err3, false) == "0xffff", true);
    ASSERT_EQ(libcpp::err_to_hex(err3, false, "err-") == "err-ffff", true);
}