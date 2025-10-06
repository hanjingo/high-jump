
#include <gtest/gtest.h>
#include <hj/db/redis.hpp>

bool _is_redis_valid()
{
    try
    {
        hj::redis r{"127.0.0.1", 6379, 2000};
        return r.is_connected();
    }
    catch(...)
    {
        return false;
    }
}

TEST(RedisTest, ConnectSetGet)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    hj::redis r{"127.0.0.1", 6379, 2000};
    ASSERT_TRUE(r.is_connected());

    ASSERT_TRUE(r.set("foo", "bar"));
    EXPECT_EQ(r.get("foo"), "bar");

    ASSERT_TRUE(r.set("foo", "baz"));
    EXPECT_EQ(r.get("foo"), "baz");

    ASSERT_TRUE(r.set("empty", ""));
    EXPECT_EQ(r.get("empty"), "");

    ASSERT_TRUE(r.set("sp#@!$", "val!@#"));
    EXPECT_EQ(r.get("sp#@!$"), "val!@#");
}

TEST(RedisTest, GetNonExist)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    hj::redis r{"127.0.0.1", 6379, 2000};
    ASSERT_TRUE(r.is_connected());
    EXPECT_EQ(r.get("not_exist_key"), "");
}

TEST(RedisTest, AuthAndSelectDb)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    hj::redis r;
    ASSERT_TRUE(r.connect("127.0.0.1", 6379, 2000, "", 1));
    ASSERT_TRUE(r.is_connected());
    ASSERT_TRUE(r.set("db1key", "v1"));
    EXPECT_EQ(r.get("db1key"), "v1");

    ASSERT_TRUE(r.connect("127.0.0.1", 6379, 2000, "", 0));
    EXPECT_EQ(r.get("db1key"), "");
}

TEST(RedisTest, InvalidConnect)
{
    hj::redis r;

    EXPECT_FALSE(r.connect("127.0.0.1", 6399, 100));

    EXPECT_FALSE(r.connect("127.0.0.1", 6379, 2000, "wrongpass"));
}

TEST(RedisTest, CmdRawInterface)
{
    if(!_is_redis_valid())
        GTEST_SKIP() << "Redis is not available";

    hj::redis r{"127.0.0.1", 6379, 2000};
    ASSERT_TRUE(r.is_connected());

    auto del = r.cmd("DEL %s", "foo");
    ASSERT_TRUE(del);

    std::string bin = std::string("\x00\x01\x02", 3);
    ASSERT_TRUE(r.set("binkey", bin));
    EXPECT_EQ(r.get("binkey"), bin);
}