#include <thread>
#include <atomic>
#include <gtest/gtest.h>
#include <hj/sync/counter.hpp>

TEST(counter, operator)
{
    hj::counter<int> ct1(2);
    hj::counter<int> ct2(3);
    hj::counter<int> ct3(2);

    EXPECT_EQ((ct1 + ct2).value(), 5);
    EXPECT_EQ((ct2 - ct1).value(), 1);
    EXPECT_EQ((ct1 * ct2).value(), 6);
    EXPECT_EQ((ct2 / ct1).value(), 1);
    // +=, -=, *=, /=
    ct1 += ct2;
    EXPECT_EQ(ct1.value(), 5);
    ct1 -= ct3;
    EXPECT_EQ(ct1.value(), 3);
    ct1 *= ct3;
    EXPECT_EQ(ct1.value(), 6);
    ct1 /= ct2;
    EXPECT_EQ(ct1.value(), 2);

    EXPECT_TRUE(ct1 == ct3);
    EXPECT_TRUE(ct1 != ct2);
    EXPECT_TRUE(ct1 < ct2);
    EXPECT_TRUE(ct2 > ct1);
    EXPECT_TRUE(ct1 <= ct3);
    EXPECT_TRUE(ct2 >= ct1);

    EXPECT_TRUE(ct1 == 2);
    EXPECT_TRUE(ct2 != 2);
    EXPECT_TRUE(ct2 > 2);
    EXPECT_TRUE(ct1 < 3);

    std::stringstream ss;
    ss << ct2;
    EXPECT_EQ(ss.str(), "3");
}

TEST(counter, inc)
{
    hj::counter<int> ct(0);
    ASSERT_EQ(ct.inc().value(), 1);
}

TEST(counter, dec)
{
    hj::counter<int> ct(1);
    ASSERT_EQ(ct.dec().value(), 0);
}

TEST(counter, step)
{
    hj::counter<int> ct(10, 0, 2); // value=10, min=0, step=2
    EXPECT_EQ(ct.step(), 2);
    ct.inc();
    EXPECT_EQ(ct.value(), 12);
    ct.dec();
    EXPECT_EQ(ct.value(), 10);
}

TEST(counter, value)
{
    hj::counter<int> ct(42);
    EXPECT_EQ(ct.value(), 42);
    ct.inc();
    EXPECT_EQ(ct.value(), 43);
    ct.dec();
    EXPECT_EQ(ct.value(), 42);
}

TEST(counter, max)
{
    hj::counter<int> ct(5, 0, 5, 1); // value=5, min=0, max=5, step=1
    EXPECT_EQ(ct.max(), 5);
    ct.inc();
    EXPECT_EQ(ct.value(), 5);
    ct.reset(10);
    EXPECT_EQ(ct.value(), 5);
}

TEST(counter, min)
{
    hj::counter<int> ct(0, 0, 5, 1); // value=0, min=0, max=5, step=1
    EXPECT_EQ(ct.min(), 0);
    ct.dec();
    EXPECT_EQ(ct.value(), 0);
    ct.reset(-1);
    EXPECT_EQ(ct.value(), 0);
}

TEST(counter, reset)
{
    hj::counter<int> ct(10, 0, 100, 1); // value=10, min=0, max=100, step=1
    ct.inc();
    ct.inc();
    EXPECT_EQ(ct.value(), 12);
    ct.reset();
    EXPECT_EQ(ct.value(), 0);
    ct.reset(5);
    EXPECT_EQ(ct.value(), 5);
    ct.reset(-1000);
    EXPECT_EQ(ct.value(), 5);
    ct.reset(200);
    EXPECT_EQ(ct.value(), 5);
}

TEST(counter, multithread_inc)
{
    constexpr int            threads    = 8;
    constexpr int            per_thread = 10000;
    hj::counter<int>         ct(0, 0, threads * per_thread, 1);
    std::vector<std::thread> ths;
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&ct, per_thread]() {
            for(int i = 0; i < per_thread; ++i)
                ct.inc();
        });
    }
    for(auto &th : ths)
        th.join();
    ASSERT_EQ(ct.value(), threads * per_thread);
}

TEST(counter, multithread_dec)
{
    constexpr int    threads    = 4;
    constexpr int    per_thread = 5000;
    hj::counter<int> ct(threads * per_thread, 0, threads * per_thread, 1);
    std::vector<std::thread> ths;
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&ct, per_thread]() {
            for(int i = 0; i < per_thread; ++i)
                ct.dec();
        });
    }
    for(auto &th : ths)
        th.join();
    ASSERT_EQ(ct.value(), 0);
}