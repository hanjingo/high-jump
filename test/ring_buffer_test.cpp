#include <gtest/gtest.h>
#include <libcpp/io/ring_buffer.hpp>

TEST(ring_buffer, push_back)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.push_back(4);
    ASSERT_EQ(buf.size(), 3);
}

TEST(ring_buffer, front)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    ASSERT_EQ(buf.front(), 1);
}

TEST(ring_buffer, back)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    ASSERT_EQ(buf.back(), 3);
}

TEST(ring_buffer, at)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    ASSERT_EQ(buf.at(0), 1);
    ASSERT_EQ(buf[0], 1);
}

TEST(ring_buffer, full)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    ASSERT_EQ(buf.full(), true);
}

TEST(ring_buffer, size)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    ASSERT_EQ(buf.size(), 2);
}

TEST(ring_buffer, capacity)
{
    libcpp::ring_buffer<int> buf{ 3 };
    buf.push_back(1);
    buf.push_back(2);
    ASSERT_EQ(buf.capacity(), 3);
}