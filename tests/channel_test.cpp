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