#include <gtest/gtest.h>
#include <hj/sync/channel.hpp>
#include <thread>

TEST(channel, wait_dequeue)
{
    static hj::channel<int> ch{1};

    ch << 0;
    std::thread t1([&]() {
        int num = 0;
        for(int i = 0; i <= 10000; ++i)
        {
            ch.wait_dequeue(num);
            num += i;
            ch << num;
        }
    });

    std::thread t2([&]() {
        int num = 0;
        for(int i = 0; i <= 10000; ++i)
        {
            ch.wait_dequeue(num);
            num += i;
            ch << num;
        }
    });

    t1.join();
    t2.join();
    int num = 0;
    ch.wait_dequeue(num);
    ASSERT_TRUE(num == 100010000);
}

TEST(channel, try_dequeue)
{
    hj::channel<int> ch{1};
    ch << 0;
    std::thread t1([&]() {
        int num = 0;
        for(int i = 0; i <= 10000; ++i)
        {
            while(!ch.try_dequeue(num))
            {
                num = 0;
            }
            num += i;
            ch << num;
        }
    });

    std::thread t2([&]() {
        int num = 0;
        for(int i = 0; i <= 10000; ++i)
        {
            while(!ch.try_dequeue(num))
            {
                num = 0;
            }
            num += i;
            ch << num;
        }
    });

    t1.join();
    t2.join();
    int num = 0;
    ASSERT_TRUE(ch.try_dequeue(num));
    ASSERT_EQ(num, 100010000);
}

TEST(channel, enqueue)
{
    static int              n = 0;
    static int              i = 0;
    static hj::channel<int> ch{2};

    std::thread t1([&]() {
        ch << 1;
        ch << 2;
        ch << 3;
    });

    std::thread t2([&]() {
        ch << 4;
        ch << 5;
    });

    ch.wait_dequeue(i);
    n += i;

    ch.wait_dequeue(i);
    n += i;

    ch.wait_dequeue(i);
    n += i;
    ;
    ch.wait_dequeue(i);
    n += i;

    ch.wait_dequeue(i);
    n += i;

    ASSERT_EQ(n, 15);
    t1.join();
    t2.join();
}

TEST(channel, construct_and_basic_ops)
{
    hj::channel<int> ch{4};
    ch << 42 << 43;
    int a = 0, b = 0;
    ch >> a >> b;
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 43);
    ch.enqueue(100);
    int c = 0;
    ASSERT_TRUE(ch.try_dequeue(c));
    ASSERT_EQ(c, 100);
}

TEST(channel, wait_dequeue_timeout)
{
    hj::channel<int> ch{2};
    int              val = 0;

    ASSERT_FALSE(ch.wait_dequeue_timeout(val, 1000));
    ch << 77;
    ASSERT_TRUE(ch.wait_dequeue_timeout(val, 1000));
    ASSERT_EQ(val, 77);
}

TEST(channel, multithread_basic)
{
    hj::channel<int>         ch{8};
    constexpr int            threads    = 4;
    constexpr int            per_thread = 1000;
    std::atomic<int>         sum{0};
    std::vector<std::thread> producers, consumers;
    for(int t = 0; t < threads; ++t)
    {
        producers.emplace_back([&ch, t, per_thread]() {
            for(int i = 0; i < per_thread; ++i)
                ch << (t * per_thread + i);
        });
    }

    for(int t = 0; t < threads; ++t)
    {
        consumers.emplace_back([&ch, &sum, per_thread]() {
            int val = 0;
            for(int i = 0; i < per_thread; ++i)
            {
                ch.wait_dequeue(val);
                sum += val;
            }
        });
    }
    for(auto &th : producers)
        th.join();
    for(auto &th : consumers)
        th.join();

    int expect = 0;
    for(int i = 0; i < threads * per_thread; ++i)
        expect += i;
    ASSERT_EQ(sum, expect);
}

TEST(channel, multithread_timeout)
{
    hj::channel<int>         ch{4};
    constexpr int            threads    = 2;
    constexpr int            per_thread = 500;
    std::atomic<int>         count{0};
    std::vector<std::thread> producers, consumers;
    for(int t = 0; t < threads; ++t)
    {
        producers.emplace_back([&ch, t, per_thread]() {
            for(int i = 0; i < per_thread; ++i)
                ch.enqueue(t * per_thread + i);
        });
    }

    for(int t = 0; t < threads; ++t)
    {
        consumers.emplace_back([&ch, &count, per_thread]() {
            int val = 0;
            for(int i = 0; i < per_thread; ++i)
            {
                if(ch.wait_dequeue_timeout(val, 10000))
                    ++count;
            }
        });
    }
    for(auto &th : producers)
        th.join();

    for(auto &th : consumers)
        th.join();

    ASSERT_EQ(count, threads * per_thread);
}