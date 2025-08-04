#include <gtest/gtest.h>
#include <libcpp/sync/thread_pool.hpp>

TEST (thread_pool, size)
{
    libcpp::thread_pool tp1{0};
    ASSERT_EQ (tp1.size (), 1);

    libcpp::thread_pool tp2{1};
    ASSERT_EQ (tp2.size (), 1);

    libcpp::thread_pool tp3{};
    ASSERT_EQ (tp3.size () > 0, true);
}

TEST (thread_pool, enqueue)
{
    libcpp::thread_pool tp{1};
    auto ret = tp.enqueue ([] () -> int { return 5; });
    ASSERT_EQ (ret.get (), 5);
}

TEST (thread_pool, bind_core_)
{
    libcpp::thread_pool tp{{1, 2}};
    auto ret = tp.enqueue ([] () -> int { return 5; });
    ASSERT_EQ (ret.get (), 5);
}

TEST (thread_pool, clear)
{
    libcpp::thread_pool tp{1};
    int n = 0;
    for (int i = 0; i < 100; ++i) {
        tp.enqueue ([&] () { n++; });
    }
    // Elegant Exit
    tp.clear ();
    ASSERT_EQ (n, 100);
}