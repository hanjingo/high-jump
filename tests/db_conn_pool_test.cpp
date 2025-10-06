
#include <gtest/gtest.h>
#include <hj/db/db_conn_pool.hpp>
#include <hj/db/sqlite.hpp>
#include <thread>
#include <vector>

TEST(db_conn_pool, capa)
{
    hj::db_conn_pool<sqlite3> pool{
        5,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("007.db", &db);
            if(ret != SQLITE_OK)
            {
                std::cout << "open db fail with err = " << sqlite3_errmsg(db)
                          << std::endl;
                return nullptr;
            }
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    ASSERT_EQ(pool.capa(), 5);
}

TEST(db_conn_pool, acquire)
{
    hj::db_conn_pool<sqlite3> pool{
        5,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("007.db", &db);
            if(ret != SQLITE_OK)
            {
                std::cout << "open db fail with err = " << sqlite3_errmsg(db)
                          << std::endl;
                return nullptr;
            }
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        },
        [](hj::db_conn_pool<sqlite3>::conn_ptr_t conn) -> bool {
            if(conn == nullptr)
                return false;
            return true;
        }};
    auto conn = pool.acquire();
    ASSERT_TRUE(conn != nullptr);
    auto conn1 = pool.acquire();
    ASSERT_TRUE(conn1 != nullptr);
    auto conn2 = pool.acquire();
    ASSERT_TRUE(conn2 != nullptr);
    auto conn3 = pool.acquire();
    ASSERT_TRUE(conn3 != nullptr);
    auto conn4 = pool.acquire();
    ASSERT_TRUE(conn4 != nullptr);
    auto conn5 = pool.acquire();
    ASSERT_TRUE(conn5 == nullptr);
}

TEST(db_conn_pool, release_and_reuse)
{
    hj::db_conn_pool<sqlite3> pool{
        2,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("008.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    auto conn1 = pool.acquire();
    auto conn2 = pool.acquire();
    ASSERT_EQ(pool.size(), 0);
    ASSERT_TRUE(conn1 != nullptr);
    ASSERT_TRUE(conn2 != nullptr);

    conn1.reset();
    ASSERT_EQ(pool.size(), 1);
    auto conn3 = pool.acquire();
    ASSERT_TRUE(conn3 != nullptr);
    conn2.reset();
    conn3.reset();
    ASSERT_EQ(pool.size(), 2);
}

TEST(db_conn_pool, timeout)
{
    hj::db_conn_pool<sqlite3> pool{
        1,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("009.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    auto conn1 = pool.acquire();
    ASSERT_TRUE(conn1 != nullptr);

    auto conn2 = pool.acquire(100);
    ASSERT_TRUE(conn2 == nullptr);
    conn1.reset();
}

TEST(db_conn_pool, check_conn)
{
    int                       fail_count = 0;
    hj::db_conn_pool<sqlite3> pool{
        2,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("010.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        },
        [&fail_count](hj::db_conn_pool<sqlite3>::conn_ptr_t conn) -> bool {
            if(conn == nullptr)
            {
                ++fail_count;
                return false;
            }
            return true;
        }};
    auto conn1 = pool.acquire();
    auto conn2 = pool.acquire();
    ASSERT_TRUE(conn1 != nullptr);
    ASSERT_TRUE(conn2 != nullptr);
    conn1.reset();
    conn2.reset();
    ASSERT_EQ(fail_count, 0);
}

TEST(db_conn_pool, concurrent_acquire_release)
{
    hj::db_conn_pool<sqlite3> pool{
        3,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("011.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    std::atomic<int>         success{0};
    std::vector<std::thread> threads;
    for(int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&]() {
            auto conn = pool.acquire(200);
            if(conn)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ++success;
            }
        });
    }
    for(auto &t : threads)
        t.join();
    ASSERT_EQ(pool.size(), 3);
    ASSERT_GE(success, 3);
}

TEST(db_conn_pool, close_and_acquire)
{
    hj::db_conn_pool<sqlite3> pool{
        2,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("012.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    auto conn1 = pool.acquire();
    pool.close();
    ASSERT_TRUE(pool.is_closed());
    auto conn2 = pool.acquire();
    ASSERT_EQ(conn2, nullptr);
    conn1.reset();
    ASSERT_EQ(pool.size(), 0);
}

TEST(db_conn_pool, acquire_after_close_multithread)
{
    hj::db_conn_pool<sqlite3> pool{
        1,
        []() -> hj::db_conn_pool<sqlite3>::conn_ptr_t {
            sqlite3 *db;
            int      ret = sqlite3_open("013.db", &db);
            if(ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3 *db) {
                sqlite3_close(db);
            });
        }};
    std::atomic<int> null_count{0};
    std::thread      t1([&]() {
        auto c = pool.acquire();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        c.reset();
    });
    std::thread      t2([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pool.close();
    });
    std::thread      t3([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        auto c = pool.acquire(100);
        if(!c)
            ++null_count;
    });
    t1.join();
    t2.join();
    t3.join();
    ASSERT_TRUE(pool.is_closed());
    ASSERT_EQ(null_count, 1);
}