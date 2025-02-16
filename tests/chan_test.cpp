#include <gtest/gtest.h>
#include <libcpp/sync/chan.hpp>

TEST(chan, wait_dequeue)
{
    int in = 1;
    int out = 0;
    libcpp::chan<int> ch;
    ch.enqueue(in);
    ch.wait_dequeue(out);
    ASSERT_EQ(out, 1);
}

TEST(chan, enqueue)
{
    int in = 1;
    libcpp::chan<int> ch;
    ASSERT_EQ(ch.enqueue(in), true);
}