#include <gtest/gtest.h>
#include <hj/sync/thread_pool.hpp>

TEST(thread_pool, size)
{
    hj::thread_pool tp1{0};
    ASSERT_EQ(tp1.size(), 1);

    hj::thread_pool tp2{1};
    ASSERT_EQ(tp2.size(), 1);

    hj::thread_pool tp3{};
    ASSERT_EQ(tp3.size() > 0, true);
}

TEST(thread_pool, enqueue)
{
    hj::thread_pool tp{1};
    auto            ret = tp.enqueue([]() -> int { return 5; });
    ASSERT_EQ(ret.get(), 5);
}

TEST(thread_pool, clear)
{
    hj::thread_pool tp{1};
    int             n = 0;
    for(int i = 0; i < 100; ++i)
    {
        tp.enqueue([&]() { n++; });
    }
    // Elegant Exit
    tp.clear();
    ASSERT_EQ(n, 100);
}

TEST(thread_pool, multi_thread_enqueue)
{
    hj::thread_pool                tp{4};
    std::atomic<int>               sum{0};
    std::vector<std::future<void>> futs;
    for(int i = 0; i < 100; ++i)
        futs.push_back(tp.enqueue([&sum] { sum++; }));
    for(auto &f : futs)
        f.get();
    ASSERT_EQ(sum, 100);
}

TEST(thread_pool, enqueue_exception)
{
    hj::thread_pool tp{1};
    auto fut = tp.enqueue([]() -> int { throw std::runtime_error("fail"); });
    ASSERT_THROW(fut.get(), std::runtime_error);
}

TEST(thread_pool, clear_then_enqueue)
{
    hj::thread_pool tp{2};
    tp.clear();
    auto fut = tp.enqueue([]() { return 1; });
    ASSERT_TRUE(fut.valid());
}

TEST(thread_pool, affinity_ctor)
{
    try
    {
        std::unordered_set<unsigned int> cores{0};
        hj::thread_pool                  tp{cores};
        ASSERT_EQ(tp.size(), 1);
    }
    catch(const std::runtime_error &e)
    {
        GTEST_SKIP() << "Affinity not supported or permission denied: "
                     << e.what();
    }
}

TEST(thread_pool, stress)
{
    hj::thread_pool                tp{8};
    std::atomic<int>               sum{0};
    std::vector<std::future<void>> futs;
    for(int i = 0; i < 1000; ++i)
        futs.push_back(tp.enqueue([&sum] { sum++; }));
    for(auto &f : futs)
        f.get();
    ASSERT_EQ(sum, 1000);
}