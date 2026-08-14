#include <gtest/gtest.h>
#include <hj/algo/gen.hpp>

#include <array>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

TEST(gen_test, gen_container_basic)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    auto             ret =
        hj::gen(buf, [](std::size_t idx) { return idx % 2 == 0 ? 2 : 1; });

    EXPECT_EQ(buf, std::vector<int>({2, 1, 2, 1, 2}));
    EXPECT_EQ(ret, buf.begin());
}

TEST(gen_test, gen_iterator_basic)
{
    std::vector<int> buf{0, 1, 2, 3, 4};
    auto             ret = hj::gen(buf.begin(), buf.end(), [](std::size_t idx) {
        return idx % 2 == 0 ? 2 : 1;
    });

    EXPECT_EQ(buf, std::vector<int>({2, 1, 2, 1, 2}));
    EXPECT_EQ(ret, buf.begin());
}

TEST(gen_test, gen_empty_container)
{
    std::vector<int> buf;
    auto             ret = hj::gen(buf, [](std::size_t idx) { return 42; });

    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(ret, buf.begin());
}

TEST(gen_test, gen_different_types)
{
    std::vector<std::string> buf(3);
    hj::gen(buf, [](std::size_t idx) { return "item_" + std::to_string(idx); });

    EXPECT_EQ(buf[0], "item_0");
    EXPECT_EQ(buf[1], "item_1");
    EXPECT_EQ(buf[2], "item_2");
}

TEST(gen_test, gen_other_containers)
{
    std::list<int> lst{0, 0, 0};
    hj::gen(lst, [](std::size_t idx) { return static_cast<int>(idx + 10); });
    EXPECT_EQ(lst, (std::list<int>{10, 11, 12}));

    std::array<int, 3> arr{0, 0, 0};
    hj::gen(arr, [](std::size_t idx) { return static_cast<int>(idx * 5); });
    EXPECT_EQ(arr, (std::array<int, 3>{0, 5, 10}));
}

TEST(gen_test, gen_stateful_lambda)
{
    std::vector<int> buf(4, 0);
    int              multiplier = 3;
    hj::gen(buf, [&](std::size_t idx) {
        return static_cast<int>(idx) * multiplier;
    });

    EXPECT_EQ(buf, std::vector<int>({0, 3, 6, 9}));
}

TEST(gen_n_test, gen_n_container_partial)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    std::size_t      filled = hj::gen_n(buf, 3, [](std::size_t idx) {
        return static_cast<int>(idx + 1);
    });

    EXPECT_EQ(filled, 3u);
    EXPECT_EQ(buf, std::vector<int>({1, 2, 3, 0, 0}));
}

TEST(gen_n_test, gen_n_container_zero_n)
{
    std::vector<int> buf{1, 2, 3, 4, 5};
    std::size_t filled = hj::gen_n(buf, 0, [](std::size_t idx) { return 99; });

    EXPECT_EQ(filled, 0u);
    EXPECT_EQ(buf, std::vector<int>({1, 2, 3, 4, 5}));
}

TEST(gen_n_test, gen_n_container_exceed_capacity)
{
    std::vector<int> buf{0, 0, 0};
    std::size_t      filled = hj::gen_n(buf, 10, [](std::size_t idx) {
        return static_cast<int>(idx + 1);
    });

    EXPECT_EQ(filled, 3u);
    EXPECT_EQ(buf, std::vector<int>({1, 2, 3}));
}

TEST(gen_n_test, gen_n_iterator_basic)
{
    std::vector<int> buf{0, 0, 0, 0, 0};
    auto             end_itr = hj::gen_n(buf.begin(), 3, [](std::size_t idx) {
        return static_cast<int>(idx + 10);
    });

    EXPECT_EQ(end_itr, buf.begin() + 3);
    EXPECT_EQ(buf, std::vector<int>({10, 11, 12, 0, 0}));
}

TEST(gen_n_test, gen_n_iterator_zero)
{
    std::vector<int> buf{1, 2, 3};
    auto end_itr = hj::gen_n(buf.begin(), 0, [](std::size_t) { return 99; });

    EXPECT_EQ(end_itr, buf.begin());
    EXPECT_EQ(buf, std::vector<int>({1, 2, 3}));
}

TEST(gen_exception_test, gen_lambda_throws)
{
    std::vector<int> buf{0, 0, 0, 0};

    EXPECT_THROW(hj::gen(buf,
                         [](std::size_t idx) -> int {
                             if(idx == 2)
                                 throw std::runtime_error("generate error");
                             return 1;
                         }),
                 std::runtime_error);

    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 0);
}

TEST(gen_exception_test, gen_n_lambda_throws)
{
    std::vector<int> buf{0, 0, 0, 0};

    EXPECT_THROW(hj::gen_n(buf,
                           4,
                           [](std::size_t idx) -> int {
                               if(idx == 1)
                                   throw std::out_of_range("out of range");
                               return 5;
                           }),
                 std::out_of_range);

    EXPECT_EQ(buf[0], 5);
    EXPECT_EQ(buf[1], 0);
}