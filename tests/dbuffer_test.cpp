#include <gtest/gtest.h>
#include <hj/sync/dbuffer.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

struct snapshot
{
    std::uint64_t                   version{};
    std::array<std::uint64_t, 1024> data{};
};

TEST(dbuffer, read)
{
    hj::dbuffer<std::vector<int>> dbuf;

    std::vector<int> buf1{1, 2, 3};
    std::vector<int> buf2{0, 0, 0};

    ASSERT_TRUE(dbuf.write(buf1));
    ASSERT_TRUE(dbuf.read(buf2));
    ASSERT_EQ(buf2, buf1);

    std::vector<int> buf3{4, 5, 6};

    ASSERT_TRUE(dbuf.write(buf3));
    ASSERT_TRUE(dbuf.read(buf2));
    ASSERT_EQ(buf2, buf3);
}

TEST(dbuffer, zero_copy_snapshot_guard)
{
    hj::dbuffer<std::string> dbuf;

    ASSERT_TRUE(dbuf.write(std::string{"first"}));

    auto guard = dbuf.read_guarded();

    ASSERT_TRUE(guard);
    ASSERT_EQ(*guard, "first");
    ASSERT_EQ(guard->size(), 5U);
    ASSERT_EQ(dbuf.active_readers(), 1U);

    ASSERT_TRUE(dbuf.write(std::string{"second"}));
    ASSERT_TRUE(dbuf.write(std::string{"third"}));

    /*
     * The old snapshot must remain valid while the guard is alive.
     */
    ASSERT_EQ(*guard, "first");
    ASSERT_GE(dbuf.retired_count(), 1U);

    /*
     * Reclamation cannot reclaim the snapshot pinned by the guard.
     */
    dbuf.reclaim();
    ASSERT_GE(dbuf.retired_count(), 1U);

    guard = {};

    ASSERT_EQ(dbuf.active_readers(), 0U);

    dbuf.reclaim();

    ASSERT_EQ(dbuf.retired_count(), 0U);

    auto current = dbuf.read_guarded();
    ASSERT_EQ(*current, "third");
}

TEST(dbuffer, guard_move)
{
    hj::dbuffer<std::string> dbuf;

    ASSERT_TRUE(dbuf.write(std::string{"hello"}));

    auto first = dbuf.read_guarded();

    ASSERT_TRUE(first);
    ASSERT_EQ(dbuf.active_readers(), 1U);

    auto second = std::move(first);

    ASSERT_FALSE(first);
    ASSERT_TRUE(second);
    ASSERT_EQ(*second, "hello");
    ASSERT_EQ(dbuf.active_readers(), 1U);

    second = {};

    ASSERT_EQ(dbuf.active_readers(), 0U);
}

TEST(dbuffer, nested_guards)
{
    hj::dbuffer<int> dbuf;

    ASSERT_TRUE(dbuf.write(42));

    auto outer = dbuf.read_guarded();

    ASSERT_EQ(dbuf.active_readers(), 1U);
    ASSERT_EQ(*outer, 42);

    {
        auto inner = dbuf.read_guarded();

        ASSERT_EQ(dbuf.active_readers(), 2U);
        ASSERT_EQ(*inner, 42);
        ASSERT_EQ(*outer, 42);
    }

    ASSERT_EQ(dbuf.active_readers(), 1U);
    ASSERT_EQ(*outer, 42);
}

TEST(dbuffer, guard_pins_old_snapshot)
{
    hj::dbuffer<int> dbuf;

    ASSERT_TRUE(dbuf.write(1));

    auto guard = dbuf.read_guarded();

    ASSERT_EQ(*guard, 1);

    for(int i = 2; i <= 100; ++i)
        ASSERT_TRUE(dbuf.write(i));

    /*
     * Zero-copy means we still access the original object rather than
     * copying it into another int.
     */
    ASSERT_EQ(*guard, 1);

    ASSERT_GT(dbuf.retired_count(), 0U);

    dbuf.reclaim();

    ASSERT_GT(dbuf.retired_count(), 0U);

    guard = {};

    dbuf.reclaim();

    ASSERT_EQ(dbuf.retired_count(), 0U);

    auto current = dbuf.read_guarded();
    ASSERT_EQ(*current, 100);
}

TEST(dbuffer, version_consistency_with_zero_copy_guard)
{
    hj::dbuffer<snapshot> dbuf;

    std::atomic<bool> stop{false};
    std::atomic<bool> failed{false};

    std::thread writer([&]() {
        std::uint64_t version = 1;

        while(!stop.load(std::memory_order_relaxed))
        {
            snapshot value;
            value.version = version;
            value.data.fill(version);

            if(!dbuf.write(value))
            {
                failed.store(true, std::memory_order_relaxed);
                break;
            }

            ++version;
        }
    });

    auto reader_func = [&]() {
        while(!stop.load(std::memory_order_relaxed))
        {
            auto guard = dbuf.read_guarded();

            if(!guard)
                continue;

            const auto version = guard->version;

            for(const auto value : guard->data)
            {
                if(value != version)
                {
                    failed.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        }
    };

    std::thread reader1(reader_func);
    std::thread reader2(reader_func);
    std::thread reader3(reader_func);
    std::thread reader4(reader_func);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    stop.store(true, std::memory_order_relaxed);

    writer.join();
    reader1.join();
    reader2.join();
    reader3.join();
    reader4.join();

    ASSERT_FALSE(failed.load(std::memory_order_relaxed));

    dbuf.reclaim();

    ASSERT_EQ(dbuf.active_readers(), 0U);
    ASSERT_EQ(dbuf.retired_count(), 0U);
}

TEST(dbuffer, read_while_writer_is_active)
{
    hj::dbuffer<snapshot> dbuf;

    std::atomic<bool> stop{false};
    std::atomic<bool> failed{false};

    std::thread writer([&]() {
        for(std::uint64_t version = 1; version <= 50000; ++version)
        {
            snapshot value;
            value.version = version;
            value.data.fill(version);

            if(!dbuf.write(value))
            {
                failed.store(true, std::memory_order_relaxed);
                break;
            }
        }

        stop.store(true, std::memory_order_relaxed);
    });

    std::vector<std::thread> readers;

    for(int i = 0; i < 8; ++i)
    {
        readers.emplace_back([&]() {
            while(!stop.load(std::memory_order_relaxed))
            {
                auto guard = dbuf.read_guarded();

                const auto version = guard->version;

                for(const auto value : guard->data)
                {
                    if(value != version)
                    {
                        failed.store(true, std::memory_order_relaxed);
                        return;
                    }
                }
            }
        });
    }

    writer.join();

    for(auto &reader : readers)
        reader.join();

    ASSERT_FALSE(failed.load(std::memory_order_relaxed));

    dbuf.reclaim();

    ASSERT_EQ(dbuf.active_readers(), 0U);
    ASSERT_EQ(dbuf.retired_count(), 0U);
}

TEST(dbuffer, MoveWrite)
{
    hj::dbuffer<std::string> dbuf;

    std::string value = "hello";

    ASSERT_TRUE(dbuf.write(std::move(value)));

    auto guard = dbuf.read_guarded();

    ASSERT_TRUE(guard);
    ASSERT_EQ(*guard, "hello");
}

TEST(dbuffer, CustomCopyFn)
{
    using V = std::vector<int>;

    struct custom_policy
    {
        bool operator()(const V &src, V &dst) const
        {
            if(src.empty())
                return false;

            dst = src;
            return true;
        }

        bool move(V &&src, V &dst) const
        {
            if(src.empty())
                return false;

            dst = std::move(src);
            return true;
        }
    };

    hj::dbuffer<V, custom_policy> dbuf;

    V v1{1, 2, 3};
    V v2;

    ASSERT_TRUE(dbuf.write(v1));
    ASSERT_TRUE(dbuf.read(v2));
    ASSERT_EQ(v2, v1);

    V v3;

    ASSERT_FALSE(dbuf.write(v3));
}

TEST(dbuffer, TypeSupport)
{
    hj::dbuffer<int> dbuf;

    int x = 42;
    int y = 0;

    ASSERT_TRUE(dbuf.write(x));
    ASSERT_TRUE(dbuf.read(y));

    ASSERT_EQ(y, 42);
}

TEST(dbuffer, ExceptionCopyFn)
{
    struct X
    {
        int v{};
    };

    struct exception_handling_policy
    {
        bool operator()(const X &src, X &dst) const noexcept
        {
            try
            {
                if(src.v == 0)
                    throw std::runtime_error("bad");

                dst = src;
                return true;
            }
            catch(...)
            {
                return false;
            }
        }
    };

    hj::dbuffer<X, exception_handling_policy> dbuf;

    X x{1};
    X y{0};

    ASSERT_TRUE(dbuf.write(x));
    ASSERT_FALSE(dbuf.write(y));

    auto guard = dbuf.read_guarded();

    ASSERT_TRUE(guard);
    ASSERT_EQ(guard->v, 1);
}

TEST(dbuffer, generation_advances_on_publish)
{
    hj::dbuffer<int> dbuf;

    const auto initial = dbuf.generation();

    ASSERT_TRUE(dbuf.write(1));
    ASSERT_EQ(dbuf.generation(), initial + 1);

    ASSERT_TRUE(dbuf.write(2));
    ASSERT_EQ(dbuf.generation(), initial + 2);
}

/*
 * This is intentionally a lightweight throughput observation rather than
 * a correctness assertion. Benchmark numbers should not be used as a
 * CI pass/fail criterion.
 */
TEST(dbuffer, benchmark_observation)
{
    constexpr int times = 100000;

    hj::dbuffer<std::vector<int>> dbuf;

    const std::vector<int> input{1, 2, 3, 4, 5, 6, 7, 8};

    auto start = std::chrono::steady_clock::now();

    std::atomic<bool> failed{false};

    std::thread writer([&]() {
        for(int i = 0; i < times; ++i)
        {
            if(!dbuf.write(input))
            {
                failed.store(true, std::memory_order_relaxed);
                return;
            }
        }
    });

    std::thread reader([&]() {
        for(int i = 0; i < times; ++i)
        {
            auto guard = dbuf.read_guarded();

            if(!guard || guard->size() != input.size())
            {
                failed.store(true, std::memory_order_relaxed);
                return;
            }
        }
    });

    writer.join();
    reader.join();

    const auto end = std::chrono::steady_clock::now();

    const auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    std::cout << "dbuffer RCU read/write: " << duration << " us" << std::endl;

    ASSERT_EQ(dbuf.active_readers(), 0U);

    dbuf.reclaim();

    ASSERT_EQ(dbuf.retired_count(), 0U);
}
