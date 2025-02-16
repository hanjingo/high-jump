#include <gtest/gtest.h>
#include <libcpp/util/init.hpp>

static int init_num = 0;

INIT(
    init_num += 1;
    ASSERT_EQ(init_num, 1);
);
INIT(
    init_num += 1;
    ASSERT_EQ(init_num, 2);
);

TEST(init, init)
{
    init_num += 1;
    ASSERT_EQ(init_num, 3);
}