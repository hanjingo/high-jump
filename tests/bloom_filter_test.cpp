#include <gtest/gtest.h>
#include <hj/algo/bloom_filter.hpp>

using namespace hj;

TEST(BloomFilterTest, BasicAddContains)
{
    bloom_filter bf(100, 0.01);
    bf.add("apple");
    bf.add("banana");
    bf.add("cherry");
    EXPECT_TRUE(bf.contains("apple"));
    EXPECT_TRUE(bf.contains("banana"));
    EXPECT_TRUE(bf.contains("cherry"));
    EXPECT_FALSE(bf.contains("durian"));
}

TEST(BloomFilterTest, Clear)
{
    bloom_filter bf(50, 0.01);
    bf.add("foo");
    EXPECT_TRUE(bf.contains("foo"));
    bf.clear();
    EXPECT_FALSE(bf.contains("foo"));
}

TEST(BloomFilterTest, SerializeUnserialize)
{
    bloom_filter bf(30, 0.01);
    bf.add("x");
    bf.add("y");
    std::string data;
    bf.serialize(data);
    bloom_filter bf2(30, 0.01);
    EXPECT_TRUE(bf2.unserialize(data));
    EXPECT_TRUE(bf2.contains("x"));
    EXPECT_TRUE(bf2.contains("y"));
    EXPECT_FALSE(bf2.contains("z"));
}