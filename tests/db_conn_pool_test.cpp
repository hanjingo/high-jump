#include <gtest/gtest.h>
#include <hj/db/db_conn_pool.hpp>
#include <sqlite3.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace
{
hj::db_conn_pool<sqlite3>::conn_ptr_t make_sqlite_mem_conn()
{
    sqlite3 *db = nullptr;
    if(sqlite3_open(":memory:", &db) != SQLITE_OK)
    {
        if(db)
            sqlite3_close(db);
        return nullptr;
    }
    return std::shared_ptr<sqlite3>(db, [](sqlite3 *d) {
        if(d)
            sqlite3_close(d);
    });
}

struct MockConn
{
    int  id{0};
    bool is_valid{true};
};
} // namespace

TEST(db_conn_pool, basic_capacity_and_statistics)
{
    constexpr std::size_t CAPA     = 5;
    constexpr std::size_t MIN_SIZE = 2;

    auto pool =
        hj::db_conn_pool<sqlite3>::create(CAPA, MIN_SIZE, make_sqlite_mem_conn);
    ASSERT_NE(pool, nullptr);

    EXPECT_EQ(pool->capa(), CAPA);
    EXPECT_EQ(pool->min_size(), MIN_SIZE);
    EXPECT_EQ(pool->idle(), MIN_SIZE);
    EXPECT_EQ(pool->active(), 0);
    EXPECT_EQ(pool->total(), MIN_SIZE);
    EXPECT_FALSE(pool->is_empty());
    EXPECT_FALSE(pool->is_closed());
}

TEST(db_conn_pool, min_size_clamping_boundary)
{
    // 故意传入 min_size(10) > capa(3)
    auto pool = hj::db_conn_pool<sqlite3>::create(3, 10, make_sqlite_mem_conn);
    ASSERT_NE(pool, nullptr);

    EXPECT_EQ(pool->capa(), 3);
    EXPECT_EQ(pool->min_size(), 3); // 应该被截断为 3
    EXPECT_EQ(pool->idle(), 3);
    EXPECT_EQ(pool->total(), 3);
}

TEST(db_conn_pool, acquire_release_and_min_size_recycling)
{
    constexpr std::size_t CAPA     = 4;
    constexpr std::size_t MIN_SIZE = 2;

    auto pool =
        hj::db_conn_pool<sqlite3>::create(CAPA, MIN_SIZE, make_sqlite_mem_conn);

    EXPECT_EQ(pool->idle(), 2);
    EXPECT_EQ(pool->active(), 0);
    EXPECT_EQ(pool->total(), 2);

    auto conn1 = pool->acquire();
    ASSERT_NE(conn1, nullptr);
    EXPECT_EQ(pool->idle(), 1);
    EXPECT_EQ(pool->active(), 1);
    EXPECT_EQ(pool->total(), 2);

    auto conn2 = pool->acquire();
    ASSERT_NE(conn2, nullptr);
    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->active(), 2);
    EXPECT_EQ(pool->total(), 2);
    EXPECT_TRUE(pool->is_empty());

    auto conn3 = pool->acquire();
    ASSERT_NE(conn3, nullptr);
    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->active(), 3);
    EXPECT_EQ(pool->total(), 3);

    auto conn4 = pool->acquire();
    ASSERT_NE(conn4, nullptr);
    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->active(), 4);
    EXPECT_EQ(pool->total(), 4);

    auto conn_fail = pool->acquire(0);
    EXPECT_EQ(conn_fail, nullptr);

    conn1.reset();
    EXPECT_EQ(pool->idle(), 1);
    EXPECT_EQ(pool->active(), 3);
    EXPECT_EQ(pool->total(), 4);

    conn2.reset();
    EXPECT_EQ(pool->idle(), 2);
    EXPECT_EQ(pool->active(), 2);
    EXPECT_EQ(pool->total(), 4);

    conn3.reset();
    EXPECT_EQ(pool->idle(), 2);
    EXPECT_EQ(pool->active(), 1);
    EXPECT_EQ(pool->total(), 3);

    conn4.reset();
    EXPECT_EQ(pool->idle(), 2);
    EXPECT_EQ(pool->active(), 0);
    EXPECT_EQ(pool->total(), 2);
}

TEST(db_conn_pool, non_blocking_acquire)
{
    auto pool = hj::db_conn_pool<sqlite3>::create(1, 1, make_sqlite_mem_conn);

    auto conn1 = pool->acquire(0);
    ASSERT_NE(conn1, nullptr);

    auto start   = std::chrono::steady_clock::now();
    auto conn2   = pool->acquire(0);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - start)
                       .count();

    EXPECT_EQ(conn2, nullptr);
    EXPECT_LT(elapsed, 20);
}

TEST(db_conn_pool, timeout_acquire)
{
    auto pool = hj::db_conn_pool<sqlite3>::create(1, 1, make_sqlite_mem_conn);

    auto conn1 = pool->acquire();
    ASSERT_NE(conn1, nullptr);

    auto start   = std::chrono::steady_clock::now();
    auto conn2   = pool->acquire(100);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - start)
                       .count();

    EXPECT_EQ(conn2, nullptr);
    EXPECT_GE(elapsed, 90);
}

TEST(db_conn_pool, infinite_wait_acquire)
{
    auto pool = hj::db_conn_pool<sqlite3>::create(1, 1, make_sqlite_mem_conn);

    auto conn1 = pool->acquire();
    ASSERT_NE(conn1, nullptr);

    std::atomic<bool>                        acquired{false};
    hj::db_conn_pool<sqlite3>::conn_handle_t c;
    std::thread                              t([&]() {
        c = pool->acquire(-1);
        if(c)
            acquired = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(acquired.load());

    conn1.reset();
    t.join();

    EXPECT_TRUE(acquired.load());
    EXPECT_NE(c, nullptr);
    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->active(), 1);
}

TEST(db_conn_pool, health_check_failure_and_recovery)
{
    int  make_counter = 0;
    auto pool         = hj::db_conn_pool<MockConn>::create(
        2,
        1,
        [&make_counter]() {
            auto conn = std::make_shared<MockConn>();
            conn->id  = ++make_counter;
            return conn;
        },
        [](std::shared_ptr<MockConn> conn) { return conn && conn->is_valid; });

    auto conn1 = pool->acquire();
    ASSERT_NE(conn1, nullptr);
    EXPECT_EQ(conn1->id, 1);

    conn1->is_valid = false;
    conn1.reset();

    auto conn2 = pool->acquire();
    ASSERT_NE(conn2, nullptr);
    EXPECT_TRUE(conn2->is_valid);
    EXPECT_GT(conn2->id, 1);
    EXPECT_EQ(pool->total(), 1);
}

TEST(db_conn_pool, health_check_throws_exception)
{
    auto pool = hj::db_conn_pool<MockConn>::create(
        1,
        0,
        []() { return std::make_shared<MockConn>(); },
        [](std::shared_ptr<MockConn>) -> bool {
            throw std::runtime_error("Check Exception");
        });

    auto conn = pool->acquire(50);
    EXPECT_EQ(conn, nullptr);
    EXPECT_EQ(pool->total(), 0);
}

TEST(db_conn_pool, make_factory_throws_or_returns_null)
{
    std::atomic<bool> should_fail{true};

    auto pool = hj::db_conn_pool<MockConn>::create(
        2,
        0,
        [&should_fail]() -> std::shared_ptr<MockConn> {
            if(should_fail.load())
            {
                throw std::runtime_error("DB Connection Failed");
            }
            return std::make_shared<MockConn>();
        });

    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->total(), 0);

    auto conn1 = pool->acquire(50);
    EXPECT_EQ(conn1, nullptr);
    EXPECT_EQ(pool->total(), 0);

    should_fail = false;

    auto conn2 = pool->acquire(100);
    ASSERT_NE(conn2, nullptr);
    EXPECT_EQ(pool->total(), 1);
    EXPECT_EQ(pool->active(), 1);
}

TEST(db_conn_pool, pool_destruction_before_handle_release)
{
    hj::db_conn_pool<sqlite3>::conn_handle_t external_conn;

    {
        auto pool =
            hj::db_conn_pool<sqlite3>::create(1, 1, make_sqlite_mem_conn);
        external_conn = pool->acquire();
        ASSERT_NE(external_conn, nullptr);
    }

    EXPECT_NO_THROW(external_conn.reset());
}

TEST(db_conn_pool, close_idempotency_and_acquire_refusal)
{
    auto pool = hj::db_conn_pool<sqlite3>::create(2, 2, make_sqlite_mem_conn);

    auto conn1 = pool->acquire();
    ASSERT_NE(conn1, nullptr);
    EXPECT_EQ(pool->total(), 2);
    EXPECT_EQ(pool->idle(), 1);

    pool->close();
    EXPECT_TRUE(pool->is_closed());
    EXPECT_EQ(pool->idle(), 0);
    EXPECT_EQ(pool->total(), 1);

    EXPECT_NO_THROW(pool->close());

    auto conn2 = pool->acquire();
    EXPECT_EQ(conn2, nullptr);

    conn1.reset();
    EXPECT_EQ(pool->total(), 0);
    EXPECT_EQ(pool->active(), 0);
    EXPECT_EQ(pool->idle(), 0);
}

TEST(db_conn_pool, high_concurrency_acquire_release)
{
    constexpr std::size_t CAPA           = 5;
    constexpr std::size_t MIN_SIZE       = 2;
    constexpr int         THREAD_COUNT   = 10;
    constexpr int         OPS_PER_THREAD = 30;

    auto pool =
        hj::db_conn_pool<sqlite3>::create(CAPA, MIN_SIZE, make_sqlite_mem_conn);

    std::atomic<int>         successful_acquires{0};
    std::atomic<int>         failed_acquires{0};
    std::vector<std::thread> threads;

    for(int i = 0; i < THREAD_COUNT; ++i)
    {
        threads.emplace_back([&]() {
            for(int j = 0; j < OPS_PER_THREAD; ++j)
            {
                auto conn = pool->acquire(20);
                if(conn)
                {
                    ++successful_acquires;
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                } else
                {
                    ++failed_acquires;
                }
            }
        });
    }

    for(auto &t : threads)
        t.join();

    EXPECT_GT(successful_acquires.load(), 0);

    EXPECT_EQ(pool->active(), 0);
    EXPECT_EQ(pool->idle(), MIN_SIZE);
    EXPECT_EQ(pool->total(), MIN_SIZE);
}