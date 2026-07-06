// ring_buffer_test.cpp
#include <gtest/gtest.h>
#include <hj/io/ring_buffer.hpp>

#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <optional>

using namespace hj;

TEST(safe_ring_buffer, constructor_validation)
{
    EXPECT_THROW(safe_ring_buffer<int> buf(0), std::invalid_argument);
    EXPECT_NO_THROW(safe_ring_buffer<int> buf(1));
    EXPECT_NO_THROW(safe_ring_buffer<int> buf(100));
}

TEST(safe_ring_buffer, empty_and_full)
{
    safe_ring_buffer<int> buf(3);

    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
    EXPECT_EQ(buf.size(), 0);
    EXPECT_EQ(buf.available(), 3);

    buf.push_back(1);
    EXPECT_FALSE(buf.empty());
    EXPECT_FALSE(buf.full());
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.available(), 2);

    buf.push_back(2);
    buf.push_back(3);
    EXPECT_FALSE(buf.empty());
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.available(), 0);
}

TEST(safe_ring_buffer, capacity)
{
    safe_ring_buffer<int> buf(5);
    EXPECT_EQ(buf.capacity(), 5);

    buf.push_back(1);
    buf.push_back(2);
    EXPECT_EQ(buf.capacity(), 5);
}

TEST(safe_ring_buffer, push_back_and_access)
{
    safe_ring_buffer<int> buf(3);

    EXPECT_TRUE(buf.try_push_back(1));
    EXPECT_TRUE(buf.try_push_back(2));
    EXPECT_TRUE(buf.try_push_back(3));
    EXPECT_FALSE(buf.try_push_back(4));

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 3);
    EXPECT_EQ(buf.at(0), 1);
    EXPECT_EQ(buf.at(1), 2);
    EXPECT_EQ(buf.at(2), 3);
    EXPECT_THROW(buf.at(3), std::out_of_range);

    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 3);
}

TEST(safe_ring_buffer, push_front)
{
    safe_ring_buffer<int> buf(3);

    buf.push_front(1);
    buf.push_front(2);
    buf.push_front(3);
    EXPECT_FALSE(buf.try_push_front(4));

    EXPECT_EQ(buf[0], 3);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 1);
    EXPECT_EQ(buf.front(), 3);
    EXPECT_EQ(buf.back(), 1);
}

TEST(safe_ring_buffer, push_back_move)
{
    safe_ring_buffer<std::string> buf(2);

    std::string s1 = "hello";
    std::string s2 = "world";

    EXPECT_TRUE(buf.try_push_back(std::move(s1)));
    EXPECT_TRUE(buf.try_push_back(std::move(s2)));
    EXPECT_FALSE(buf.try_push_back(std::string("test")));

    EXPECT_EQ(buf[0], "hello");
    EXPECT_EQ(buf[1], "world");
    EXPECT_TRUE(s1.empty());
    EXPECT_TRUE(s2.empty());
}

TEST(safe_ring_buffer, push_front_move)
{
    safe_ring_buffer<std::string> buf(2);

    std::string s1 = "hello";
    std::string s2 = "world";

    EXPECT_TRUE(buf.try_push_front(std::move(s1)));
    EXPECT_TRUE(buf.try_push_front(std::move(s2)));
    EXPECT_FALSE(buf.try_push_front(std::string("test")));

    EXPECT_EQ(buf[0], "world");
    EXPECT_EQ(buf[1], "hello");
}

TEST(safe_ring_buffer, pop_front)
{
    safe_ring_buffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    auto v1 = buf.try_pop_front();
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(*v1, 1);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0], 2);

    auto v2 = buf.try_pop_front();
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 2);
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf[0], 3);

    auto v3 = buf.try_pop_front();
    ASSERT_TRUE(v3.has_value());
    EXPECT_EQ(*v3, 3);
    EXPECT_TRUE(buf.empty());

    auto v4 = buf.try_pop_front();
    EXPECT_FALSE(v4.has_value());
}

TEST(safe_ring_buffer, pop_back)
{
    safe_ring_buffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    auto v1 = buf.try_pop_back();
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(*v1, 3);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[1], 2);

    auto v2 = buf.try_pop_back();
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 2);
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf[0], 1);

    auto v3 = buf.try_pop_back();
    ASSERT_TRUE(v3.has_value());
    EXPECT_EQ(*v3, 1);
    EXPECT_TRUE(buf.empty());

    auto v4 = buf.try_pop_back();
    EXPECT_FALSE(v4.has_value());
}

TEST(safe_ring_buffer, insert)
{
    safe_ring_buffer<int> buf(5);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(4);

    EXPECT_TRUE(buf.try_insert(2, 3));
    EXPECT_EQ(buf.size(), 4);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 3);
    EXPECT_EQ(buf[3], 4);

    EXPECT_TRUE(buf.try_insert(0, 0));
    EXPECT_EQ(buf.size(), 5);
    EXPECT_EQ(buf[0], 0);
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 2);
    EXPECT_EQ(buf[3], 3);
    EXPECT_EQ(buf[4], 4);

    EXPECT_FALSE(buf.try_insert(0, 5));
    EXPECT_FALSE(buf.try_insert(5, 5));

    EXPECT_FALSE(buf.try_insert(6, 5));
}

TEST(safe_ring_buffer, insert_move)
{
    safe_ring_buffer<std::string> buf(3);
    buf.push_back("one");
    buf.push_back("three");

    std::string s = "two";
    EXPECT_TRUE(buf.try_insert(1, std::move(s)));

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], "one");
    EXPECT_EQ(buf[1], "two");
    EXPECT_EQ(buf[2], "three");
    EXPECT_TRUE(s.empty());
}

TEST(safe_ring_buffer, erase)
{
    safe_ring_buffer<int> buf(5);
    for(int i = 0; i < 5; ++i)
    {
        buf.push_back(i);
    }

    buf.erase(2);
    EXPECT_EQ(buf.size(), 4);
    EXPECT_EQ(buf[0], 0);
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 3);
    EXPECT_EQ(buf[3], 4);

    buf.erase(0);
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 3);
    EXPECT_EQ(buf[2], 4);

    buf.erase(2);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 3);

    buf.erase(10);
    EXPECT_EQ(buf.size(), 2);
}

TEST(safe_ring_buffer, clear)
{
    safe_ring_buffer<int> buf(5);
    for(int i = 0; i < 5; ++i)
    {
        buf.push_back(i);
    }

    EXPECT_FALSE(buf.empty());
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0);
    EXPECT_EQ(buf.available(), 5);

    buf.push_back(42);
    EXPECT_EQ(buf[0], 42);
}

TEST(safe_ring_buffer, push_back_batch)
{
    safe_ring_buffer<int> buf(5);
    std::vector<int>      data = {1, 2, 3, 4, 5, 6, 7};

    size_t count = buf.push_back_batch(data, data.size());
    EXPECT_EQ(count, 5);
    EXPECT_EQ(buf.size(), 5);
    EXPECT_TRUE(buf.full());

    for(size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(buf[i], static_cast<int>(i + 1));
    }
}

TEST(safe_ring_buffer, push_back_batch_with_timeout)
{
    safe_ring_buffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    std::vector<int> data = {4, 5, 6};

    size_t count = buf.push_back_batch(data, data.size(), 10);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(buf.size(), 3);
}

TEST(safe_ring_buffer, push_front_batch)
{
    safe_ring_buffer<int> buf(5);
    std::vector<int>      data = {1, 2, 3, 4, 5, 6, 7};

    size_t count = buf.push_front_batch(data, data.size());
    EXPECT_EQ(count, 5);
    EXPECT_EQ(buf.size(), 5);

    for(size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(buf[i], static_cast<int>(5 - i));
    }
}

TEST(safe_ring_buffer, pop_front_batch)
{
    safe_ring_buffer<int> buf(5);
    for(int i = 0; i < 5; ++i)
    {
        buf.push_back(i);
    }

    std::vector<int> output(3);
    size_t           count = buf.pop_front_batch(output, output.size());

    EXPECT_EQ(count, 3);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(output[0], 0);
    EXPECT_EQ(output[1], 1);
    EXPECT_EQ(output[2], 2);
    EXPECT_EQ(buf[0], 3);
    EXPECT_EQ(buf[1], 4);
}

TEST(safe_ring_buffer, pop_front_batch_all)
{
    safe_ring_buffer<int> buf(3);
    for(int i = 0; i < 3; ++i)
    {
        buf.push_back(i);
    }

    std::vector<int> output(5);
    size_t           count = buf.pop_front_batch(output, output.size());

    EXPECT_EQ(count, 3);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(output[0], 0);
    EXPECT_EQ(output[1], 1);
    EXPECT_EQ(output[2], 2);
}

TEST(safe_ring_buffer, pop_back_batch)
{
    safe_ring_buffer<int> buf(5);
    for(int i = 0; i < 5; ++i)
    {
        buf.push_back(i);
    }

    std::vector<int> output(3);
    size_t           count = buf.pop_back_batch(output, output.size());

    EXPECT_EQ(count, 3);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(output[0], 4);
    EXPECT_EQ(output[1], 3);
    EXPECT_EQ(output[2], 2);
    EXPECT_EQ(buf[0], 0);
    EXPECT_EQ(buf[1], 1);
}

TEST(safe_ring_buffer, pop_front_batch_empty)
{
    safe_ring_buffer<int> buf(3);
    std::vector<int>      output(3);

    size_t count = buf.pop_front_batch(output, output.size(), 10);
    EXPECT_EQ(count, 0);
}

TEST(safe_ring_buffer, move_constructor)
{
    safe_ring_buffer<int> buf1(3);
    buf1.push_back(1);
    buf1.push_back(2);
    buf1.push_back(3);

    safe_ring_buffer<int> buf2(std::move(buf1));

    EXPECT_EQ(buf2.size(), 3);
    EXPECT_EQ(buf2[0], 1);
    EXPECT_EQ(buf2[1], 2);
    EXPECT_EQ(buf2[2], 3);
    EXPECT_EQ(buf2.capacity(), 3);
}

TEST(safe_ring_buffer, move_assignment)
{
    safe_ring_buffer<int> buf1(3);
    buf1.push_back(1);
    buf1.push_back(2);
    buf1.push_back(3);

    safe_ring_buffer<int> buf2(5);
    buf2.push_back(4);
    buf2.push_back(5);

    buf2 = std::move(buf1);

    EXPECT_EQ(buf2.size(), 3);
    EXPECT_EQ(buf2[0], 1);
    EXPECT_EQ(buf2[1], 2);
    EXPECT_EQ(buf2[2], 3);
    EXPECT_EQ(buf2.capacity(), 3);
}

TEST(safe_ring_buffer, move_assignment_self)
{
    safe_ring_buffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf = std::move(buf);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 3);
}

TEST(safe_ring_buffer, swap)
{
    safe_ring_buffer<int> buf1(3);
    buf1.push_back(1);
    buf1.push_back(2);
    buf1.push_back(3);

    safe_ring_buffer<int> buf2(2);
    buf2.push_back(4);
    buf2.push_back(5);

    buf1.swap(buf2);

    EXPECT_EQ(buf1.size(), 2);
    EXPECT_EQ(buf1[0], 4);
    EXPECT_EQ(buf1[1], 5);
    EXPECT_EQ(buf1.capacity(), 2);

    EXPECT_EQ(buf2.size(), 3);
    EXPECT_EQ(buf2[0], 1);
    EXPECT_EQ(buf2[1], 2);
    EXPECT_EQ(buf2[2], 3);
    EXPECT_EQ(buf2.capacity(), 3);
}

struct NonCopyable
{
    int value;
    NonCopyable(int v)
        : value(v)
    {
    }
    NonCopyable(const NonCopyable &)            = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&)                 = default;
    NonCopyable &operator=(NonCopyable &&)      = default;
};

TEST(safe_ring_buffer, non_copyable_type)
{
    safe_ring_buffer<NonCopyable> buf(2);

    NonCopyable obj1(1);
    NonCopyable obj2(2);

    EXPECT_TRUE(buf.try_push_back(std::move(obj1)));
    EXPECT_TRUE(buf.try_push_back(std::move(obj2)));

    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0].value, 1);
    EXPECT_EQ(buf[1].value, 2);

    auto v = buf.try_pop_front();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->value, 1);
}

TEST(safe_ring_buffer, push_back_blocking)
{
    safe_ring_buffer<int> buf(2);
    buf.push_back(1);
    buf.push_back(2);

    EXPECT_FALSE(buf.try_push_back(3));
}

TEST(safe_ring_buffer, pop_front_blocking)
{
    safe_ring_buffer<int> buf(2);

    auto v = buf.try_pop_front();
    EXPECT_FALSE(v.has_value());
}

TEST(safe_ring_buffer, push_back_timeout)
{
    safe_ring_buffer<int> buf(1);
    buf.push_back(1);

    bool result = buf.push_back(2, 10);
    EXPECT_FALSE(result);
}

TEST(safe_ring_buffer, pop_front_timeout)
{
    safe_ring_buffer<int> buf(1);

    auto v = buf.pop_front(10);
    EXPECT_FALSE(v.has_value());
}

TEST(safe_ring_buffer, single_producer_single_consumer)
{
    const int             ITERATIONS = 1000;
    const int             CAPACITY   = 100;
    safe_ring_buffer<int> buf(CAPACITY);

    std::atomic<bool> done{false};
    std::vector<int>  received;
    received.reserve(ITERATIONS);

    std::thread consumer([&]() {
        while(true)
        {
            auto v = buf.pop_front(10);
            if(v.has_value())
            {
                received.push_back(*v);
            } else if(done)
            {
                while(auto v2 = buf.try_pop_front())
                {
                    received.push_back(*v2);
                }
                break;
            }
        }
    });

    std::thread producer([&]() {
        for(int i = 0; i < ITERATIONS; ++i)
        {
            buf.push_back(i);
        }
        done = true;
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(received.size(), ITERATIONS);
    for(int i = 0; i < ITERATIONS; ++i)
    {
        EXPECT_EQ(received[i], i);
    }
}

TEST(safe_ring_buffer, multi_producer_multi_consumer)
{
    const int PRODUCERS          = 4;
    const int CONSUMERS          = 4;
    const int ITEMS_PER_PRODUCER = 500;
    const int TOTAL_ITEMS        = PRODUCERS * ITEMS_PER_PRODUCER;
    const int CAPACITY           = 50;

    safe_ring_buffer<int> buf(CAPACITY);
    std::atomic<int>      produced{0};
    std::atomic<int>      consumed{0};
    std::vector<int>      received;
    std::mutex            received_mutex;

    std::vector<std::thread> producers;
    for(int p = 0; p < PRODUCERS; ++p)
    {
        producers.emplace_back([&, p]() {
            for(int i = 0; i < ITEMS_PER_PRODUCER; ++i)
            {
                buf.push_back(p * ITEMS_PER_PRODUCER + i);
                produced.fetch_add(1);
            }
        });
    }

    std::vector<std::thread> consumers;
    std::atomic<bool>        done{false};

    for(int c = 0; c < CONSUMERS; ++c)
    {
        consumers.emplace_back([&]() {
            std::vector<int> local;
            local.reserve(TOTAL_ITEMS / CONSUMERS);

            while(true)
            {
                auto v = buf.pop_front(10);
                if(v.has_value())
                {
                    local.push_back(*v);
                    consumed.fetch_add(1);
                } else if(done && consumed >= TOTAL_ITEMS)
                {
                    break;
                }
            }

            std::lock_guard<std::mutex> lock(received_mutex);
            received.insert(received.end(), local.begin(), local.end());
        });
    }

    for(auto &t : producers)
    {
        t.join();
    }
    done = true;

    for(auto &t : consumers)
    {
        t.join();
    }

    EXPECT_EQ(produced, TOTAL_ITEMS);
    EXPECT_EQ(consumed, TOTAL_ITEMS);
    EXPECT_EQ(received.size(), TOTAL_ITEMS);

    std::sort(received.begin(), received.end());
    for(int i = 0; i < TOTAL_ITEMS; ++i)
    {
        EXPECT_EQ(received[i], i);
    }
}

TEST(safe_ring_buffer, producer_faster_than_consumer)
{
    const int             ITERATIONS = 500;
    const int             CAPACITY   = 20;
    safe_ring_buffer<int> buf(CAPACITY);
    std::atomic<int>      consumed_count{0};

    std::thread consumer([&]() {
        for(int i = 0; i < ITERATIONS; ++i)
        {
            auto v = buf.pop_front();
            EXPECT_TRUE(v.has_value());
            if(v.has_value())
            {
                EXPECT_EQ(*v, i);
                consumed_count++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::thread producer([&]() {
        for(int i = 0; i < ITERATIONS; ++i)
        {
            buf.push_back(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumed_count, ITERATIONS);
}

TEST(safe_ring_buffer, consumer_faster_than_producer)
{
    const int             ITERATIONS = 200;
    const int             CAPACITY   = 20;
    safe_ring_buffer<int> buf(CAPACITY);
    std::atomic<int>      consumed_count{0};
    std::atomic<bool>     done{false};

    std::thread consumer([&]() {
        while(!done || !buf.empty())
        {
            auto v = buf.pop_front(10);
            if(v.has_value())
            {
                consumed_count++;
            }
        }
    });

    std::thread producer([&]() {
        for(int i = 0; i < ITERATIONS; ++i)
        {
            buf.push_back(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        done = true;
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumed_count, ITERATIONS);
}

TEST(safe_ring_buffer, batch_operations_with_threads)
{
    const int             BATCH_SIZE = 10;
    const int             BATCHES    = 20;
    const int             CAPACITY   = 30;
    safe_ring_buffer<int> buf(CAPACITY);
    std::atomic<int>      total_consumed{0};
    std::atomic<bool>     done{false};

    std::thread consumer([&]() {
        std::vector<int> buffer(BATCH_SIZE);

        while(!done || !buf.empty())
        {
            size_t count = buf.pop_front_batch(buffer, BATCH_SIZE, 10);
            if(count > 0)
            {
                total_consumed += static_cast<int>(count);
            }
        }
    });

    std::thread producer([&]() {
        std::vector<int> data(BATCH_SIZE);

        for(int b = 0; b < BATCHES; ++b)
        {
            for(int i = 0; i < BATCH_SIZE; ++i)
            {
                data[i] = b * BATCH_SIZE + i;
            }

            size_t inserted = 0;
            while(inserted < data.size())
            {
                size_t count = buf.push_back_batch(data, data.size(), 100);
                inserted += count;
                if(count == 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
        done = true;
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(total_consumed, BATCH_SIZE * BATCHES);
}

struct ThrowOnCopy
{
    int         value;
    static bool should_throw;

    ThrowOnCopy(int v = 0)
        : value(v)
    {
    }

    ThrowOnCopy(const ThrowOnCopy &other)
    {
        if(should_throw)
        {
            throw std::runtime_error("Copy constructor throws");
        }
        value = other.value;
    }

    ThrowOnCopy(ThrowOnCopy &&other) noexcept
        : value(other.value)
    {
    }

    ThrowOnCopy &operator=(const ThrowOnCopy &other)
    {
        if(should_throw)
        {
            throw std::runtime_error("Copy assignment throws");
        }
        value = other.value;
        return *this;
    }

    ThrowOnCopy &operator=(ThrowOnCopy &&other) noexcept
    {
        value = other.value;
        return *this;
    }
};

bool ThrowOnCopy::should_throw = false;

TEST(safe_ring_buffer, exception_safety_push)
{
    safe_ring_buffer<ThrowOnCopy> buf(3);
    ThrowOnCopy::should_throw = false;

    ThrowOnCopy obj1(1);
    ThrowOnCopy obj2(2);
    buf.push_back(obj1);
    buf.push_back(obj2);

    EXPECT_EQ(buf.size(), 2);

    ThrowOnCopy::should_throw = true;

    ThrowOnCopy obj3(3);
    EXPECT_THROW(buf.push_back(obj3), std::runtime_error);

    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0].value, 1);
    EXPECT_EQ(buf[1].value, 2);
}

TEST(safe_ring_buffer, exception_safety_try_push)
{
    safe_ring_buffer<ThrowOnCopy> buf(3);
    ThrowOnCopy::should_throw = false;

    ThrowOnCopy obj1(1);
    ThrowOnCopy obj2(2);
    buf.push_back(obj1);
    buf.push_back(obj2);

    EXPECT_EQ(buf.size(), 2);

    ThrowOnCopy::should_throw = true;

    ThrowOnCopy obj3(3);
    EXPECT_THROW(buf.try_push_back(obj3), std::runtime_error);

    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0].value, 1);
    EXPECT_EQ(buf[1].value, 2);
}

TEST(safe_ring_buffer, custom_type)
{
    struct Point
    {
        int x, y;
        Point(int a = 0, int b = 0)
            : x(a)
            , y(b)
        {
        }
        bool operator==(const Point &other) const
        {
            return x == other.x && y == other.y;
        }
    };

    safe_ring_buffer<Point> buf(3);
    buf.push_back(Point(1, 2));
    buf.push_back(Point(3, 4));
    buf.push_back(Point(5, 6));

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], Point(1, 2));
    EXPECT_EQ(buf[1], Point(3, 4));
    EXPECT_EQ(buf[2], Point(5, 6));

    auto p = buf.try_pop_front();
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(*p, Point(1, 2));
}

TEST(safe_ring_buffer, large_data)
{
    const int             SIZE = 1000;
    safe_ring_buffer<int> buf(SIZE);

    for(int i = 0; i < SIZE; ++i)
    {
        buf.push_back(i);
    }

    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.size(), SIZE);
    EXPECT_EQ(buf.front(), 0);
    EXPECT_EQ(buf.back(), SIZE - 1);

    std::vector<int> output(SIZE);
    size_t           count = buf.pop_front_batch(output, SIZE);
    EXPECT_EQ(count, SIZE);

    for(int i = 0; i < SIZE; ++i)
    {
        EXPECT_EQ(output[i], i);
    }
}

TEST(safe_ring_buffer, capacity_one)
{
    safe_ring_buffer<int> buf(1);

    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
    EXPECT_EQ(buf.available(), 1);

    buf.push_back(42);
    EXPECT_FALSE(buf.empty());
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.available(), 0);
    EXPECT_EQ(buf[0], 42);

    EXPECT_FALSE(buf.try_push_back(43));
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf[0], 42);

    auto v = buf.try_pop_front();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 42);
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
    EXPECT_EQ(buf.available(), 1);
}

TEST(safe_ring_buffer, full_capacity)
{
    safe_ring_buffer<int> buf(10);
    EXPECT_EQ(buf.capacity(), 10);
    EXPECT_EQ(buf.available(), 10);

    for(int i = 0; i < 10; ++i)
    {
        buf.push_back(i);
    }
    EXPECT_EQ(buf.available(), 0);

    for(int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(buf[i], i);
    }
}

TEST(safe_ring_buffer, wrap_around)
{
    safe_ring_buffer<int> buf(3);

    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.try_pop_front();
    buf.try_pop_front();
    buf.push_back(4);
    buf.push_back(5);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 3);
    EXPECT_EQ(buf[1], 4);
    EXPECT_EQ(buf[2], 5);

    buf.try_pop_front();
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0], 4);
    EXPECT_EQ(buf[1], 5);
}

TEST(safe_ring_buffer, alternating_push_pop)
{
    safe_ring_buffer<int> buf(5);

    for(int i = 0; i < 100; ++i)
    {
        buf.push_back(i);
        auto v = buf.try_pop_front();
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(*v, i);
        EXPECT_TRUE(buf.empty());
    }
}

TEST(safe_ring_buffer, front_back_on_single_element)
{
    safe_ring_buffer<int> buf(3);
    buf.push_back(42);

    EXPECT_EQ(buf.front(), 42);
    EXPECT_EQ(buf.back(), 42);

    buf.try_pop_front();
    EXPECT_TRUE(buf.empty());
}

TEST(safe_ring_buffer, available_after_ops)
{
    safe_ring_buffer<int> buf(5);

    EXPECT_EQ(buf.available(), 5);

    buf.push_back(1);
    EXPECT_EQ(buf.available(), 4);

    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.available(), 2);

    buf.try_pop_front();
    EXPECT_EQ(buf.available(), 3);

    buf.clear();
    EXPECT_EQ(buf.available(), 5);
}