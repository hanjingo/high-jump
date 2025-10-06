#include <gtest/gtest.h>
#include <hj/io/ring_buffer.hpp>

TEST(ring_buffer, push_back_overwrite)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.push_back(4);
    ASSERT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 2);
    EXPECT_EQ(buf[1], 3);
    EXPECT_EQ(buf[2], 4);
}

TEST(ring_buffer, front)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.front(), 1);
    buf.push_back(4);
    EXPECT_EQ(buf.front(), 2);
}

TEST(ring_buffer, back)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.back(), 3);
    buf.push_back(4);
    EXPECT_EQ(buf.back(), 4);
}

TEST(ring_buffer, at_and_operator)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.at(0), 1);
    EXPECT_EQ(buf[0], 1);
    buf.push_back(4);
    EXPECT_EQ(buf.at(0), 2);
    EXPECT_EQ(buf[0], 2);

    EXPECT_THROW(buf.at(3), std::out_of_range);
}

TEST(ring_buffer, full_and_empty)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_FALSE(buf.full());
    buf.push_back(3);
    EXPECT_TRUE(buf.full());
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
}

TEST(ring_buffer, size_and_clear)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_EQ(buf.size(), 0);
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_EQ(buf.size(), 2);
    buf.push_back(3);
    EXPECT_EQ(buf.size(), 3);
    buf.clear();
    EXPECT_EQ(buf.size(), 0);
}

TEST(ring_buffer, capacity)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_EQ(buf.capacity(), 3);
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_EQ(buf.capacity(), 3);
}

TEST(ring_buffer, pop_front_and_pop_back)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.pop_front();
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), 2);
    buf.pop_back();
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.back(), 2);
}

TEST(ring_buffer, iterator)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    int sum = 0;
    for(auto v : buf)
        sum += v;
    EXPECT_EQ(sum, 6);
    buf.push_back(4); // now 2,3,4
    std::vector<int> v(buf.begin(), buf.end());
    EXPECT_EQ((v == std::vector<int>({2, 3, 4})), true);
}