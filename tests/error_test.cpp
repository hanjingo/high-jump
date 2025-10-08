#include <gtest/gtest.h>
#include <hj/testing/error.hpp>

enum class err1
{
    ok = -1,
    fail,
    unknown,
};

enum err2
{
    err2_ok       = 0,
    err2_io_fail  = 1,
    err2_net_fail = 1001,
    err2_db_fail  = 2001,
};

enum err3
{
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

TEST(error, reg_and_make_err)
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

TEST(error, reg_and_make_nested_err)
{
    hj::register_err("io", err2_io_fail, "read/write io error");
    hj::register_err("net", err2_net_fail, "network error");
    hj::register_err("db", err2_db_fail, "db error");

    auto nested1 = hj::make_err(hj::make_err(err2_net_fail, "net"),
                                hj::make_err(err2_io_fail, "io"));
    auto nested2 = hj::make_err(hj::make_err(err2_db_fail, "db"),
                                hj::make_err(err2_io_fail, "io"));

    ASSERT_EQ(nested1.ec.value(), 1001);
    ASSERT_EQ(nested1.ec.category().name(), std::string("net"));
    ASSERT_EQ(nested1.ec.message(), std::string("network error"));
    ASSERT_NE(nested1.cause, nullptr);
    ASSERT_EQ(nested1.cause->ec.value(), 1);
    ASSERT_EQ(nested1.cause->ec.category().name(), std::string("io"));
    ASSERT_EQ(nested1.cause->ec.message(), std::string("read/write io error"));
    ASSERT_EQ(nested1.cause->cause, nullptr);

    ASSERT_EQ(nested2.ec.value(), 2001);
    ASSERT_EQ(nested2.ec.category().name(), std::string("db"));
    ASSERT_EQ(nested2.ec.message(), std::string("db error"));
    ASSERT_NE(nested2.cause, nullptr);
    ASSERT_EQ(nested2.cause->ec.value(), 1);
    ASSERT_EQ(nested2.cause->ec.category().name(), std::string("io"));
    ASSERT_EQ(nested2.cause->ec.message(), std::string("read/write io error"));
    ASSERT_EQ(nested2.cause->cause, nullptr);
}

TEST(error, concurrent_category)
{
    constexpr int            N = 8;
    std::vector<std::thread> threads;
    for(int i = 0; i < N; ++i)
    {
        threads.emplace_back([i] {
            std::string cat = "cat" + std::to_string(i);
            hj::register_err(cat.c_str(), i, "desc" + std::to_string(i));
            auto ec = hj::make_err(i, cat.c_str());
            ASSERT_EQ(ec.value(), i);
            ASSERT_EQ(ec.category().name(), cat);
            ASSERT_EQ(ec.message(), "desc" + std::to_string(i));
        });
    }
    for(auto &t : threads)
        t.join();
}

TEST(error, equivalent)
{
    hj::register_err("test", 123, "test error");
    std::error_code      ec = hj::make_err(123, "test");
    std::error_condition cond(123, ec.category());
    ASSERT_TRUE(ec.category().equivalent(123, cond));
}

TEST(error, unknown_message)
{
    std::error_code ec = hj::make_err(9999, "not_exist");
    ASSERT_EQ(ec.message(), "unknown error");
}

TEST(error, nested_chain_traverse)
{
    hj::register_err("a", 1, "err_a");
    hj::register_err("b", 2, "err_b");
    hj::register_err("c", 3, "err_c");
    auto n3    = hj::make_err(3, "c");
    auto n2    = hj::make_err(2, "b");
    auto n1    = hj::make_err(1, "a");
    auto chain = hj::err_detail::nested_error_code(
        n1,
        std::make_shared<hj::err_detail::nested_error_code>(
            n2,
            std::make_shared<hj::err_detail::nested_error_code>(n3)));
    ASSERT_EQ(chain.ec.message(), "err_a");
    ASSERT_EQ(chain.cause->ec.message(), "err_b");
    ASSERT_EQ(chain.cause->cause->ec.message(), "err_c");
}
