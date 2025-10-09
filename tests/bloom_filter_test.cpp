#include <gtest/gtest.h>
#include <hj/algo/bloom_filter.hpp>

using namespace hj;

TEST(bloom_filter, basic_add_contains)
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

TEST(bloom_filter, clear)
{
    bloom_filter bf(50, 0.01);
    bf.add("foo");
    EXPECT_TRUE(bf.contains("foo"));
    bf.clear();
    EXPECT_FALSE(bf.contains("foo"));
}

TEST(bloom_filter, serialize_unserialize)
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

TEST(bloom_filter, parameter_validation)
{
    bloom_filter bf0(0, 0.01);
    EXPECT_GE(bf0.bit_size(), BLOOM_FILTER_MIN_BITS);
    EXPECT_GE(bf0.hash_count(), BLOOM_FILTER_MIN_HASHES);

    bloom_filter bf1(100, -1.0);
    EXPECT_GE(bf1.bit_size(), BLOOM_FILTER_MIN_BITS);
    EXPECT_GE(bf1.hash_count(), BLOOM_FILTER_MIN_HASHES);

    bloom_filter bf2(100, 2.0);
    EXPECT_GE(bf2.bit_size(), BLOOM_FILTER_MIN_BITS);
    EXPECT_GE(bf2.hash_count(), BLOOM_FILTER_MIN_HASHES);
}

TEST(bloom_filter, empty_string)
{
    bloom_filter bf(50, 0.01);

#ifndef NDEBUG
    EXPECT_DEATH(bf.add(""), "value must not be empty");
    EXPECT_DEATH(bf.contains(""), "value must not be empty");
#endif
}

TEST(bloom_filter, serialize_unserialize_invalid)
{
    bloom_filter bf(20, 0.01);
    std::string  invalid_data = "101";
    EXPECT_FALSE(bf.unserialize(invalid_data));
}

TEST(bloom_filter, copy_and_move)
{
    bloom_filter bf1(40, 0.01);
    bf1.add("a");
    bf1.add("b");
    bloom_filter bf2 = bf1;
    EXPECT_TRUE(bf2.contains("a"));
    EXPECT_TRUE(bf2.contains("b"));
    bloom_filter bf3(40, 0.01);
    bf3 = bf1;
    EXPECT_TRUE(bf3.contains("a"));
    EXPECT_TRUE(bf3.contains("b"));
}

TEST(bloom_filter, type_support_int)
{
    bloom_filter<int> bf(30, 0.01);
    bf.add(42);
    bf.add(7);
    EXPECT_TRUE(bf.contains(42));
    EXPECT_TRUE(bf.contains(7));
    EXPECT_FALSE(bf.contains(99));
}