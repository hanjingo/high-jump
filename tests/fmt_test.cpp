#include <gtest/gtest.h>
#include <hj/encoding/fmt.hpp>

TEST(fmt, fmt)
{
    ASSERT_EQ(hj::fmt("{}-{}", "hello", "world") == std::string("hello-world"),
              true);
    ASSERT_EQ(hj::fmt("{1}-{0}", "hello", "world")
                  == std::string("world-hello"),
              true);
}