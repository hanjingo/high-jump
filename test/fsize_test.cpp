#include <gtest/gtest.h>
#include <libcpp/io/file.hpp>

TEST(file, BYTE)
{
    ASSERT_EQ(BYTE(1), 1);
}

TEST(file, KB)
{
    ASSERT_EQ(KB(1), 1 * 1024);
}

TEST(file, MB)
{
    ASSERT_EQ(MB(1), 1 * 1024 * 1024);
}

TEST(file, GB)
{
    ASSERT_EQ(GB(1), 1 * 1024 * 1024 * 1024);
}

TEST(file, TB)
{
    ASSERT_EQ(TB(1) == (unsigned long long)(1) * 1024 * 1024 * 1024 * 1024,
              true);
}