#include <gtest/gtest.h>
#include <hj/algo/fill.hpp>
#include <vector>

TEST(fill, fill)
{
    // fill
    std::vector<int> buf{0, 0, 0, 0, 0};
    hj::fill(buf, [](const unsigned long idx) -> int {
        if(idx % 2 == 0)
            return 2;
        else
            return 1;
    });

    ASSERT_EQ(buf[0], 2);
    ASSERT_EQ(buf[1], 1);
    ASSERT_EQ(buf[2], 2);
    ASSERT_EQ(buf[3], 1);
    ASSERT_EQ(buf[4], 2);

    // fill iterator
    std::vector<int> buf1{0, 1, 2, 3, 4};
    hj::fill(buf1.begin(), buf1.end(), [](const unsigned long idx) -> int {
        if(idx % 2 == 0)
            return 2;
        else
            return 1;
    });

    ASSERT_EQ(buf1[0], 2);
    ASSERT_EQ(buf1[1], 1);
    ASSERT_EQ(buf1[2], 2);
    ASSERT_EQ(buf1[3], 1);
    ASSERT_EQ(buf1[4], 2);

    // fill n
    std::vector<int> buf2{0, 0, 0, 0, 0};
    hj::fill(
        buf2,
        [](const unsigned long idx) -> int {
            if(idx % 2 == 0)
                return 2;
            else
                return 1;
        },
        4);

    ASSERT_EQ(buf2[0], 2);
    ASSERT_EQ(buf2[1], 1);
    ASSERT_EQ(buf2[2], 2);
    ASSERT_EQ(buf2[3], 1);
    ASSERT_EQ(buf2[4], 0);
}