#include <gtest/gtest.h>
#include <hj/util/once.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <stdexcept>

void test_once_function(int *counter)
{
    HJ_ONCE(*counter = 1;);
}

TEST(once, simple_execution)
{
    static int counter = 0;

    counter = 0;

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);

    test_once_function(&counter);
    EXPECT_EQ(counter, 1);
}

TEST(once, once_in_loop)
{
    static int value = 0;

    value = 0;

    for(int i = 0; i < 5; ++i)
    {
        HJ_ONCE(value = 42;);
    }

    EXPECT_EQ(value, 42);
}

TEST(once, complex_expression)
{
    static int a = 0, b = 0;

    a = b = 0;

    HJ_ONCE(a = 10; b = 20;);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20);

    for(int i = 0; i < 3; ++i)
    {
        HJ_ONCE(a = 100; b = 200;);
    }

    EXPECT_EQ(a, 100);
    EXPECT_EQ(b, 200);
}

TEST(once, thread_safety)
{
    static std::atomic<int> counter{0};
    counter.store(0);

    auto worker = []() { HJ_ONCE(counter++;); };

    std::vector<std::thread> threads;
    threads.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
        threads.emplace_back(worker);
    }
    for(auto &t : threads)
    {
        t.join();
    }

    EXPECT_EQ(counter.load(), 1);
}

TEST(once, exception_retry)
{
    static int attempts = 0;
    attempts            = 0;

    auto try_once = [&]() {
        HJ_ONCE(attempts++; if(attempts < 3) {
            throw std::runtime_error("initialization failed");
        });
    };

    EXPECT_THROW(try_once(), std::runtime_error);
    EXPECT_EQ(attempts, 1);

    EXPECT_THROW(try_once(), std::runtime_error);
    EXPECT_EQ(attempts, 2);

    EXPECT_NO_THROW(try_once());
    EXPECT_EQ(attempts, 3);

    EXPECT_NO_THROW(try_once());
    EXPECT_EQ(attempts, 3);
}