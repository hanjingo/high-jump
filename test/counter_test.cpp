#include <gtest/gtest.h>
#include <libcpp/sync/counter.hpp>

TEST(counter, operator) {}

TEST(counter, inc)
{
    libcpp::counter<int> ct(0);
    ASSERT_EQ(ct.inc().value(), 1);
}

TEST(counter, dec)
{
    libcpp::counter<int> ct(1);
    ASSERT_EQ(ct.dec().value(), 0);
}

TEST(counter, step) {}

TEST(counter, value) {}

TEST(counter, max) {}

TEST(counter, min) {}

TEST(counter, reset) {}