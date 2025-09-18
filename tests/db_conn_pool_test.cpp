#include <gtest/gtest.h>
#include <libcpp/db/db_conn_pool.hpp>

#include <libcpp/db/sqlite.hpp>

TEST(db_conn_pool, capa)
{
    libcpp::db_conn_pool<sqlite3> pool{5, []()->libcpp::db_conn_pool<sqlite3>::conn_ptr_t{
        sqlite3* db;
        int ret = sqlite3_open("007.db", &db);
        if (ret != SQLITE_OK) 
        {
            std::cout << "open db fail with err = " << sqlite3_errmsg(db) << std::endl;
            return nullptr;
        }
        return std::shared_ptr<sqlite3>(db, [](sqlite3* db) {
            sqlite3_close(db);
        });
    }};
    ASSERT_EQ(pool.capa(), 5);
}

TEST(db_conn_pool, acquire)
{
    libcpp::db_conn_pool<sqlite3> pool{
        5, 
        []()->libcpp::db_conn_pool<sqlite3>::conn_ptr_t{
            sqlite3* db;
            int ret = sqlite3_open("007.db", &db);
            if (ret != SQLITE_OK) 
            {
                std::cout << "open db fail with err = " << sqlite3_errmsg(db) << std::endl;
                return nullptr;
            }
            return std::shared_ptr<sqlite3>(db, [](sqlite3* db) {
                sqlite3_close(db);
            });
        },
        [](libcpp::db_conn_pool<sqlite3>::conn_ptr_t conn)->bool {
            if (conn == nullptr)
                return false;
            return true;
        }};
    auto conn = pool.acquire();
    ASSERT_EQ(conn != nullptr, true);
    auto conn1 = pool.acquire();
    ASSERT_EQ(conn1 != nullptr, true);
    auto conn2 = pool.acquire();
    ASSERT_EQ(conn2 != nullptr, true);
    auto conn3 = pool.acquire();
    ASSERT_EQ(conn3 != nullptr, true);
    auto conn4 = pool.acquire();
    ASSERT_EQ(conn4 != nullptr, true);
    auto conn5 = pool.acquire();
    ASSERT_EQ(conn5 == nullptr, true);
}

TEST(db_conn_pool, release_and_reuse)
{
    libcpp::db_conn_pool<sqlite3> pool{
        2,
        []()->libcpp::db_conn_pool<sqlite3>::conn_ptr_t{
            sqlite3* db;
            int ret = sqlite3_open("008.db", &db);
            if (ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3* db){ sqlite3_close(db); });
        }
    };
    auto conn1 = pool.acquire();
    auto conn2 = pool.acquire();
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(conn1 != nullptr, true);
    ASSERT_EQ(conn2 != nullptr, true);

    conn1.reset();
    ASSERT_EQ(pool.size(), 1);
    auto conn3 = pool.acquire();
    ASSERT_EQ(conn3 != nullptr, true);
    conn2.reset();
    conn3.reset();
    ASSERT_EQ(pool.size(), 2);
}

TEST(db_conn_pool, timeout)
{
    libcpp::db_conn_pool<sqlite3> pool{
        1,
        []()->libcpp::db_conn_pool<sqlite3>::conn_ptr_t{
            sqlite3* db;
            int ret = sqlite3_open("009.db", &db);
            if (ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3* db){ sqlite3_close(db); });
        }
    };
    auto conn1 = pool.acquire();
    ASSERT_EQ(conn1 != nullptr, true);

    auto conn2 = pool.acquire(100);
    ASSERT_EQ(conn2 == nullptr, true);
    conn1.reset();
}

TEST(db_conn_pool, check_conn)
{
    int fail_count = 0;
    libcpp::db_conn_pool<sqlite3> pool{
        2,
        []()->libcpp::db_conn_pool<sqlite3>::conn_ptr_t{
            sqlite3* db;
            int ret = sqlite3_open("010.db", &db);
            if (ret != SQLITE_OK)
                return nullptr;
            return std::shared_ptr<sqlite3>(db, [](sqlite3* db){ sqlite3_close(db); });
        },
        [&fail_count](libcpp::db_conn_pool<sqlite3>::conn_ptr_t conn)->bool {
            if (conn == nullptr) {
                ++fail_count;
                return false;
            }
            return true;
        }
    };
    auto conn1 = pool.acquire();
    auto conn2 = pool.acquire();
    ASSERT_EQ(conn1 != nullptr, true);
    ASSERT_EQ(conn2 != nullptr, true);
    conn1.reset();
    conn2.reset();
    ASSERT_EQ(fail_count, 0);
}