
#include <gtest/gtest.h>
#include <libcpp/db/redis.hpp>

bool _is_redis_valid()
{
    try {
        libcpp::redis r{"127.0.0.1", 6379, 2000};
        return r.is_connected();
    } catch (...) {
        return false;
    }
}

TEST(RedisTest, ConnectSetGet) 
{
    if (!_is_redis_valid()) 
        GTEST_SKIP() << "Redis is not available";

	libcpp::redis r{"127.0.0.1", 6379, 2000};
	ASSERT_TRUE(r.is_connected());
	ASSERT_TRUE(r.set("foo", "bar"));
	EXPECT_EQ(r.get("foo"), "bar");
}

TEST(RedisTest, GetNonExist) 
{
    if (!_is_redis_valid()) 
        GTEST_SKIP() << "Redis is not available";
        
	libcpp::redis r{"127.0.0.1", 6379, 2000};
	ASSERT_TRUE(r.is_connected());
	EXPECT_EQ(r.get("not_exist_key"), "");
}