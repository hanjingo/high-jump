#include <gtest/gtest.h>
#include <libcpp/util/once.hpp>

static int once_num = 0;

void inc()
{
    ONCE(
        once_num += 1;
    )
}

TEST(once, once)
{
    inc();
    inc();
    inc();
    ASSERT_EQ(once_num, 1);
}
