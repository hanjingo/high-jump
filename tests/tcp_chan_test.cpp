#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>

TEST(tcp_chan, wait_dequeue)
{
    static libcpp::tcp_chan<int> ch{1};
    
    static int n = 0;
    std::thread t1([&](){
        int i = 0;
        ch >> i;
        n += i;
        ch >> i;
        n += i;
        ch >> i;
        n += i;
    });

    std::thread t2([&](){
        int i = 0;
        ch >> i;
        n += i;
        ch >> i;
        n += i;
    });

    ch << 1;
    ch << 2;
    ch << 3;
    ch << 4;
    ch << 5;

    t1.join();
    t2.join();
    ASSERT_EQ(n == 15, true);
}

TEST(tcp_chan, try_dequeue)
{
    libcpp::tcp_chan<int> ch{1};
    ch << 0;
    std::thread t1([&](){
        int num = 0;
        for (int i = 0; i <= 10000; ++i)
        {
            while (!ch.try_dequeue(num)) 
            {
                num = 0;
            }
            num += i;
            ch << num;
        }
    });

    std::thread t2([&](){
        int num = 0;
        for (int i = 0; i <= 10000; ++i)
        {
            while (!ch.try_dequeue(num)) 
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
    ASSERT_EQ(ch.try_dequeue(num), true);
    ASSERT_EQ(num, 100010000);
}

TEST(tcp_chan, enqueue)
{
    static int n = 0;
    static int i = 0;
    static libcpp::tcp_chan<int> ch{2};

    std::thread t1([&](){
        ch << 1;
        ch << 2;
        ch << 3;
    });

    std::thread t2([&](){
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