#include <gtest/gtest.h>
#include <hj/db/redis.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

using namespace hj;

static bool _is_redis_valid()
{
    redis r{"127.0.0.1", 6379, 1000};
    return r.is_connected();
}

TEST(redis, connect_and_move)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r1("127.0.0.1", 6379, 1000);
    ASSERT_TRUE(r1.is_connected());

    redis r2(std::move(r1));
    EXPECT_FALSE(r1.is_connected());
    EXPECT_TRUE(r2.is_connected());

    redis r3;
    r3 = std::move(r2);
    EXPECT_FALSE(r2.is_connected());
    EXPECT_TRUE(r3.is_connected());

    r3.close();
    EXPECT_FALSE(r3.is_connected());
}

TEST(redis, exec_and_query_types)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r{"127.0.0.1", 6379, 1000};
    ASSERT_TRUE(r.is_connected());
    db_conn &conn = r;

    conn.exec("DEL key_str key_int key_blob");

    std::string_view   s_in       = "hello_redis";
    int64_t            i_in       = 9876543210LL;
    uint8_t            raw_blob[] = {0x00, 0xFF, 0x12, 0x00, 0xAB};
    db_conn::blob_view b_in{raw_blob, sizeof(raw_blob)};

    auto res1 = conn.exec("SET ? ?", "key_str", s_in);
    EXPECT_FALSE(res1.ec);
    EXPECT_EQ(res1.affected_rows, 1);

    auto res2 = conn.exec("SET ? ?", "key_int", i_in);
    EXPECT_FALSE(res2.ec);

    auto res3 = conn.exec("SET ? ?", "key_blob", b_in);
    EXPECT_FALSE(res3.ec);

    db_conn::ret_t outs;
    auto           err = conn.query(outs, "GET ?", "key_str");
    EXPECT_FALSE(err);
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "hello_redis");

    outs = db_conn::ret_t{};
    err  = conn.query(outs, "MGET key_str key_int key_blob key_nonexist");
    EXPECT_FALSE(err);
    ASSERT_EQ(outs.rows(), 4u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "hello_redis");
    EXPECT_EQ(outs.get_or<std::string>(1, 0, ""), "9876543210");

    std::string blob_str = outs.get_or<std::string>(2, 0, "");
    EXPECT_EQ(blob_str.size(), sizeof(raw_blob));
    EXPECT_EQ(std::memcmp(blob_str.data(), raw_blob, sizeof(raw_blob)), 0);

    EXPECT_TRUE(outs.is_null(3, 0));

    conn.exec("DEL key_str key_int key_blob");
}

TEST(redis, transaction_manual)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r{"127.0.0.1", 6379, 1000};
    ASSERT_TRUE(r.is_connected());
    db_conn &conn = r;

    conn.exec("DEL trans_k1 trans_k2");

    EXPECT_FALSE(conn.begin());
    conn.exec("SET trans_k1 v1");
    EXPECT_FALSE(conn.rollback());

    db_conn::ret_t outs;
    conn.query(outs, "GET trans_k1");
    EXPECT_TRUE(outs.empty() || outs.is_null(0, 0));

    EXPECT_FALSE(conn.begin());
    conn.exec("SET trans_k2 v2");
    EXPECT_FALSE(conn.commit());

    conn.query(outs, "GET trans_k2");
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "v2");

    conn.exec("DEL trans_k1 trans_k2");
}

TEST(redis, trans_guard)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r{"127.0.0.1", 6379, 1000};
    ASSERT_TRUE(r.is_connected());
    db_conn &conn = r;

    conn.exec("DEL guard_k1 guard_k2");

    {
        db_conn::trans_guard guard(conn);
        conn.exec("SET guard_k1 should_rollback");
    }

    db_conn::ret_t outs;
    conn.query(outs, "GET guard_k1");
    EXPECT_TRUE(outs.empty() || outs.is_null(0, 0));

    {
        db_conn::trans_guard guard(conn);
        conn.exec("SET guard_k2 should_commit");
        EXPECT_FALSE(guard.commit());
    }

    conn.query(outs, "GET guard_k2");
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "should_commit");

    conn.exec("DEL guard_k1 guard_k2");
}

TEST(redis, auth_and_select_db)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r;

    ASSERT_TRUE(r.connect("127.0.0.1", 6379, 1000, "", 1));
    ASSERT_TRUE(r.is_connected());
    r.exec("SET db1key v1");

    ASSERT_TRUE(r.connect("127.0.0.1", 6379, 1000, "", 0));
    db_conn::ret_t outs;
    r.query(outs, "GET db1key");
    EXPECT_TRUE(outs.empty() || outs.is_null(0, 0));

    r.connect("127.0.0.1", 6379, 1000, "", 1);
    r.exec("DEL db1key");
}

TEST(redis, error_handling)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    redis r{"127.0.0.1", 6379, 1000};
    ASSERT_TRUE(r.is_connected());

    auto res = r.exec("INVALID_REDIS_COMMAND_NAME");
    EXPECT_TRUE(res.ec);
    EXPECT_FALSE(r.get_last_error().empty());

    auto res_param = r.exec("SET only_one_arg");
    EXPECT_TRUE(res_param.ec);

    r.close();
    auto res_closed = r.exec("PING");
    EXPECT_EQ(res_closed.ec, std::errc::not_connected);
}

TEST(redis, invalid_connect)
{
    redis r;
    EXPECT_FALSE(r.connect("127.0.0.1", 6399, 100));

    EXPECT_FALSE(r.connect("127.0.0.1", 6379, 1000, "wrong_password_123456"));
}