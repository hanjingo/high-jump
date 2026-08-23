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
    ASSERT_TRUE(ch >> a);
    ASSERT_TRUE(ch >> b);
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
                if(ch.wait_dequeue_timeout(val, 5000))
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

struct move_only
{
    int id;
    explicit move_only(int i)
        : id(i)
    {
    }
    ~move_only() = default;

    move_only(const move_only &)            = delete;
    move_only &operator=(const move_only &) = delete;

    move_only(move_only &&)            = default;
    move_only &operator=(move_only &&) = default;
};

TEST(channel, move_only_type_support)
{
    hj::channel<move_only> ch{2};

    ch.enqueue(move_only(42));
    ch.emplace(100);

    move_only m1(0);
    ASSERT_TRUE(ch.wait_dequeue(m1));
    ASSERT_EQ(m1.id, 42);

    move_only m2(0);
    ASSERT_TRUE(ch.wait_dequeue(m2));
    ASSERT_EQ(m2.id, 100);
}

TEST(channel, capacity_semantics_as_initial_allocation)
{
    hj::channel<int> ch{1};

    for(int i = 0; i < 100; ++i)
    {
        ch << i;
    }

    ASSERT_EQ(ch.size(), 100);

    int val = 0;
    for(int i = 0; i < 100; ++i)
    {
        ASSERT_TRUE(ch >> val);
        ASSERT_EQ(val, i);
    }
}

TEST(channel, empty_channel_operator_right_shift_blocks)
{
    hj::channel<int>  ch{4};
    std::atomic<bool> received{false};

    std::thread consumer([&ch, &received]() {
        int val = -1;
        ASSERT_TRUE(ch >> val);
        ASSERT_EQ(val, 999);
        received = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(received);

    ch << 999;

    consumer.join();
    ASSERT_TRUE(received);
}

TEST(channel, ExtremeStressTest8P8C1M)
{
    hj::channel<uint64_t> ch{1024};

    constexpr int      num_producers         = 8;
    constexpr int      num_consumers         = 8;
    constexpr uint64_t messages_per_producer = 125000;
    constexpr uint64_t total_messages = num_producers * messages_per_producer;

    std::vector<std::atomic<int>> receipt_tracker(total_messages);
    for(auto &count : receipt_tracker)
    {
        count.store(0, std::memory_order_relaxed);
    }

    std::atomic<uint64_t> total_consumed{0};

    std::vector<std::thread> producers;
    producers.reserve(num_producers);
    for(int p = 0; p < num_producers; ++p)
    {
        producers.emplace_back([&ch, p, messages_per_producer]() {
            uint64_t start_id = p * messages_per_producer;
            for(uint64_t i = 0; i < messages_per_producer; ++i)
            {
                ch << (start_id + i);
            }
        });
    }

    std::vector<std::thread> consumers;
    consumers.reserve(num_consumers);
    for(int c = 0; c < num_consumers; ++c)
    {
        consumers.emplace_back(
            [&ch, &receipt_tracker, &total_consumed, total_messages]() {
                uint64_t val = 0;
                while(total_consumed.load(std::memory_order_relaxed)
                      < total_messages)
                {
                    if(ch.wait_dequeue(val))
                    {
                        if(val < total_messages)
                        {
                            receipt_tracker[val].fetch_add(
                                1,
                                std::memory_order_relaxed);
                        }
                        total_consumed.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
    }

    for(auto &th : producers)
    {
        th.join();
    }

    while(total_consumed.load(std::memory_order_relaxed) < total_messages)
    {
        std::this_thread::yield();
    }

    ch.close();

    for(auto &th : consumers)
    {
        th.join();
    }

    uint64_t lost_messages      = 0;
    uint64_t duplicate_messages = 0;

    for(uint64_t i = 0; i < total_messages; ++i)
    {
        int count = receipt_tracker[i].load(std::memory_order_relaxed);
        if(count == 0)
        {
            lost_messages++;
        } else if(count > 1)
        {
            duplicate_messages++;
        }
    }

    ASSERT_EQ(lost_messages, 0) << "Error: Detected lost messages!";
    ASSERT_EQ(duplicate_messages, 0) << "Error: Detected duplicate messages!";
    ASSERT_EQ(total_consumed.load(), total_messages);
}

TEST(ChannelLifecycle, ConsumerStartsLate)
{
    hj::channel<int>  ch{4};
    std::atomic<bool> consumed{false};

    std::thread consumer([&ch, &consumed]() {
        int val = 0;
        ASSERT_TRUE(ch.wait_dequeue(val));
        ASSERT_EQ(val, 888);
        consumed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(consumed);

    ch << 888;
    consumer.join();
    ASSERT_TRUE(consumed);
}

TEST(ChannelLifecycle, ProducerStartsLate)
{
    hj::channel<int> ch{4};
    ch << 123;
    ch << 456;

    std::thread consumer([&ch]() {
        int val1 = 0, val2 = 0;
        ASSERT_TRUE(ch.wait_dequeue(val1));
        ASSERT_TRUE(ch.wait_dequeue(val2));
        ASSERT_EQ(val1, 123);
        ASSERT_EQ(val2, 456);
    });

    consumer.join();
}

TEST(ChannelLifecycle, MultipleProducersFinishAndConsumerDrains)
{
    hj::channel<int> ch{8};
    constexpr int    num_producers      = 3;
    constexpr int    items_per_producer = 100;

    std::vector<std::thread> producers;
    for(int p = 0; p < num_producers; ++p)
    {
        producers.emplace_back([&ch, p, items_per_producer]() {
            for(int i = 0; i < items_per_producer; ++i)
            {
                ch << (p * 1000 + i);
            }
        });
    }

    for(auto &th : producers)
    {
        th.join();
    }

    ch.close();

    std::thread consumer([&]() {
        int val   = 0;
        int count = 0;
        while(ch.wait_dequeue(val))
        {
            count++;
        }
        ASSERT_EQ(count, num_producers * items_per_producer);
    });

    consumer.join();
}

TEST(ChannelLifecycle, ChannelDestructionSafety)
{
    {
        hj::channel<std::string> ch{2};
        ch << "hello" << "world";
    }
    SUCCEED();
}