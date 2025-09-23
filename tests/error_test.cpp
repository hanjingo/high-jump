#include <gtest/gtest.h>
#include <hj/testing/error.hpp>

enum class err1
{
    ok = -1,
    fail,
    unknown,
};

enum err2 {
    err2_ok = 0,
    err2_net_fail = 1001,
    err2_db_fail = 2001,
};

enum err3 {
    err3_e1 = 10,
    err3_e2 = 20
};

TEST(error, ec_to_int)
{
    ASSERT_EQ(hj::ec_to_int(err1::ok) == -1, true);
    ASSERT_EQ(hj::ec_to_int(err1::fail) == 0, true);
    ASSERT_EQ(hj::ec_to_int(err1::unknown) == 1, true);

    ASSERT_EQ(hj::ec_to_int(err1::ok, 1) == 1, true);
    ASSERT_EQ(hj::ec_to_int(err1::fail, 1) == 0, true);
    ASSERT_EQ(hj::ec_to_int(err1::unknown, 1) == 1, true);
}

TEST(error, ec_to_hex)
{
    ASSERT_EQ(hj::ec_to_hex(err1::ok) == "0xFFFFFFFF", true);
    ASSERT_EQ(hj::ec_to_hex(err1::ok, false) == "0xffffffff", true);
    ASSERT_EQ(hj::ec_to_hex(err1::ok, false, "err-") == "err-ffffffff", true);
    ASSERT_EQ(hj::ec_to_hex(err1::fail) == "0x0", true);
    ASSERT_EQ(hj::ec_to_hex(err1::unknown) == "0x1", true);

    std::uint16_t err2 = 1024;
    ASSERT_EQ(hj::ec_to_hex(err2) == "0x400", true);

    std::uint16_t err3 = 65535;
    ASSERT_EQ(hj::ec_to_hex(err3) == "0xFFFF", true);
    ASSERT_EQ(hj::ec_to_hex(err3, false) == "0xffff", true);
    ASSERT_EQ(hj::ec_to_hex(err3, false, "err-") == "err-ffff", true);
}

TEST(error, reg_err_and_make_err)
{
    hj::register_err("net", err2_net_fail, "network error");
    hj::register_err("db", err2_db_fail, "db error");

    std::error_code ec1 = hj::make_err(err2_net_fail, "net");
    std::error_code ec2 = hj::make_err(err2_db_fail, "db");
    std::error_code ec3 = hj::make_err(9999, "net");

    ASSERT_EQ(ec1.value(), 1001);
    ASSERT_EQ(ec1.category().name(), std::string("net"));
    ASSERT_EQ(ec1.message(), std::string("network error"));

    ASSERT_EQ(ec2.value(), 2001);
    ASSERT_EQ(ec2.category().name(), std::string("db"));
    ASSERT_EQ(ec2.message(), std::string("db error"));

    ASSERT_EQ(ec3.message(), std::string("unknown error"));
}