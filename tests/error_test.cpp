#include <gtest/gtest.h>
#include <hj/testing/error.hpp>
#include <cstdint>
#include <thread>
#include <type_traits>
#include <vector>

enum class NetError
{
    ok                 = 0,
    io_fail            = 1,
    net_fail           = 1001,
    connection_refused = 1002,
    timeout            = 1003,
};

HJ_REG_ERR_CATEGORY(NetError, NetErrorCategory, "net")

std::string NetErrorCategory::message(int ev) const
{
    switch(static_cast<NetError>(ev))
    {
        case NetError::ok:
            return "ok";
        case NetError::io_fail:
            return "read/write io error";
        case NetError::net_fail:
            return "network error";
        case NetError::connection_refused:
            return "connection refused";
        case NetError::timeout:
            return "timeout";
        default:
            return "unknown error";
    }
}

std::error_condition
NetErrorCategory::default_error_condition(int ev) const noexcept
{
    switch(static_cast<NetError>(ev))
    {
        case NetError::ok:
            return hj::generic_errc::success;
        case NetError::io_fail:
            return hj::generic_errc::io_failure;
        case NetError::net_fail:
        case NetError::connection_refused:
        case NetError::timeout:
            return hj::generic_errc::network_failure;
        default:
            return std::error_category::default_error_condition(ev);
    }
}

enum class DbError
{
    ok      = 0,
    db_fail = 2001,
};

HJ_REG_ERR_CATEGORY(DbError, DbErrorCategory, "db")

std::string DbErrorCategory::message(int ev) const
{
    switch(static_cast<DbError>(ev))
    {
        case DbError::ok:
            return "ok";
        case DbError::db_fail:
            return "db error";
        default:
            return "unknown error";
    }
}

std::error_condition
DbErrorCategory::default_error_condition(int ev) const noexcept
{
    if(static_cast<DbError>(ev) == DbError::ok)
        return hj::generic_errc::success;
    return hj::generic_errc::resource_unavailable;
}

enum class HugeError : uint64_t
{
    ok       = 0,
    huge_val = 0xFFFFFFFFFFFFFFFFULL
};

enum class EnumInt8 : int8_t
{
    neg = -1,
    pos = 127
};

enum class EnumUint8 : uint8_t
{
    max_val = 255
};

enum class PlainEnum
{
    e1 = -1,
    e2 = 0,
    e3 = 1
};


TEST(error, error_condition_matching)
{
    std::error_code ec_refused = NetError::connection_refused;
    std::error_code ec_timeout = NetError::timeout;
    std::error_code ec_io      = NetError::io_fail;

    EXPECT_EQ(ec_refused, hj::generic_errc::network_failure);
    EXPECT_EQ(ec_timeout, hj::generic_errc::network_failure);

    EXPECT_NE(ec_io, hj::generic_errc::network_failure);
    EXPECT_EQ(ec_io, hj::generic_errc::io_failure);

    std::error_code db_ec = DbError::db_fail;
    EXPECT_EQ(db_ec, hj::generic_errc::resource_unavailable);
}

TEST(error, unknown_error)
{
    std::error_code ec{99999, get_NetErrorCategory()};
    EXPECT_EQ(ec.message(), "unknown error");
}

TEST(error, zero_error)
{
    std::error_code ec = NetError::ok;
    EXPECT_FALSE(ec);
    EXPECT_EQ(ec.value(), 0);
    EXPECT_EQ(ec, hj::generic_errc::success);
}

TEST(error, category_equality)
{
    std::error_code ec1 = NetError::net_fail;
    std::error_code ec2 = NetError::io_fail;

    EXPECT_EQ(ec1.category(), ec2.category());
    EXPECT_EQ(&ec1.category(), &ec2.category());
}

TEST(error, different_categories)
{
    std::error_code net_ec{1, get_NetErrorCategory()};
    std::error_code db_ec{1, get_DbErrorCategory()};

    EXPECT_NE(net_ec, db_ec);
    EXPECT_NE(net_ec.category(), db_ec.category());
}

TEST(error, copy_move_semantics)
{
    hj::nested_error a{NetError::net_fail, NetError::io_fail};
    hj::nested_error b = a;            // Copy Construct
    hj::nested_error c = std::move(b); // Move Construct

    EXPECT_EQ(c.ec, NetError::net_fail);
    ASSERT_NE(c.cause, nullptr);
    EXPECT_EQ(c.cause->ec, NetError::io_fail);

    EXPECT_EQ(a.ec, NetError::net_fail);
}

TEST(error, deep_chain)
{
    constexpr int    depth = 1000;
    hj::nested_error current{NetError::io_fail};

    for(int i = 0; i < depth; ++i)
    {
        current =
            hj::make_nested_error(NetError::net_fail,
                                  std::make_shared<hj::nested_error>(current));
    }

    int  count = 0;
    auto node  = std::make_shared<hj::nested_error>(current);
    while(node)
    {
        count++;
        node = node->cause;
    }
    EXPECT_EQ(count, depth + 1);
}

TEST(error, integral_types_conversion)
{
    int8_t  i8 = -1;
    uint8_t u8 = 255;
    EXPECT_EQ(hj::ec_to_hex(i8), "0xFF");
    EXPECT_EQ(hj::ec_to_hex(u8), "0xFF");
    EXPECT_EQ(hj::ec_to_hex(EnumInt8::neg), "0xFF");
    EXPECT_EQ(hj::ec_to_hex(EnumUint8::max_val), "0xFF");

    int64_t  i64 = -1;
    uint64_t u64 = 0xFFFFFFFFFFFFFFFFULL;
    EXPECT_EQ(hj::ec_to_hex(i64), "0xFFFFFFFFFFFFFFFF");
    EXPECT_EQ(hj::ec_to_hex(u64), "0xFFFFFFFFFFFFFFFF");
    EXPECT_EQ(hj::ec_to_hex(HugeError::huge_val), "0xFFFFFFFFFFFFFFFF");

    EXPECT_EQ(hj::to_underlying(HugeError::huge_val), 0xFFFFFFFFFFFFFFFFULL);
}

TEST(error, compile_time_tests)
{
    static_assert(hj::ec_to_int(PlainEnum::e1) == -1,
                  "constexpr ec_to_int failed");
    static_assert(hj::ec_to_int(PlainEnum::e2) == 0,
                  "constexpr ec_to_int failed");
    static_assert(hj::to_underlying(PlainEnum::e1) == -1,
                  "constexpr to_underlying failed");

    SUCCEED();
}

TEST(error, noexcept_specifications)
{
    NetError         err = NetError::net_fail;
    hj::nested_error n_err;

    static_assert(noexcept(make_error_code(err)),
                  "make_error_code must be noexcept!");
    static_assert(noexcept(hj::to_underlying(err)),
                  "to_underlying must be noexcept!");
    static_assert(noexcept(hj::ec_to_int(err)), "ec_to_int must be noexcept!");
    static_assert(noexcept(static_cast<bool>(n_err)),
                  "operator bool must be noexcept!");

    SUCCEED();
}