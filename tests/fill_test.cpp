#include <gtest/gtest.h>
#include <hj/algo/fill.hpp>
#include <vector>

TEST(fill, fill_basic)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    hj::fill(buf, [](size_t idx) { return idx % 2 == 0 ? 2 : 1; });
    ASSERT_EQ(buf, std::vector<int>({2, 1, 2, 1, 2}));
}

TEST(fill, fill_iterator)
{
    std::vector<int> buf{0, 1, 2, 3, 4};
    hj::fill(buf.begin(), buf.end(), [](size_t idx) {
        return idx % 2 == 0 ? 2 : 1;
    });
    ASSERT_EQ(buf, std::vector<int>({2, 1, 2, 1, 2}));
}

TEST(fill, fill_n)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    hj::fill(buf, [](size_t idx) { return idx % 2 == 0 ? 2 : 1; }, 4);
    ASSERT_EQ(buf, std::vector<int>({2, 1, 2, 1, 0}));
}

TEST(fill, empty_container)
{
    std::vector<int> buf;
    hj::fill(buf, [](size_t idx) { return 42; });
    ASSERT_TRUE(buf.empty());
}

TEST(fill, fill_partial)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    hj::fill(buf, [](size_t idx) { return 7; }, 2);
    ASSERT_EQ(buf, std::vector<int>({7, 7, 0, 0, 0}));
}

TEST(fill, fill_different_type)
{
    std::vector<std::string> buf(3);
    hj::fill(buf, [](size_t idx) { return std::to_string(idx); });
    ASSERT_EQ(buf[0], "0");
    ASSERT_EQ(buf[1], "1");
    ASSERT_EQ(buf[2], "2");
}

TEST(fill, fill_exception_lambda)
{
    std::vector<int> buf{0, 0, 0};
    EXPECT_THROW(hj::fill(buf,
                          [](size_t idx) -> int {
                              if(idx == 1)
                                  throw std::runtime_error("fail");
                              return 1;
                          }),
                 std::runtime_error);
}