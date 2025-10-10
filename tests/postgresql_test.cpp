#include <gtest/gtest.h>
#include <hj/db/postgresql.hpp>
#include <string>
#include <vector>

// NOTE: Requires a running PostgreSQL server and a test database/user.
// Example connection string: "host=localhost port=5432 dbname=testdb user=testuser password=testpass"

const std::string conninfo =
    "host=localhost port=5432 dbname=postgres user=postgres password=postgres";

static bool _is_pg_server_running()
{
    // Check if PostgreSQL server is running
    try
    {
        hj::pg_connection conn(
            "dbname=postgres user=postgres password=postgres");
        if(conn.is_open())
        {
            conn.disconnect();
            return true;
        }
    }
    catch(const std::exception &e)
    {
        // Handle exception (e.g., log it)
    }
    return false;
}


TEST(postgresql, connect_and_disconnect)
{
    if(!_is_pg_server_running())
        GTEST_SKIP()
            << "PostgreSQL server is not running or connection failed.";

    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());
    db.disconnect();
    ASSERT_FALSE(db.is_open());
    // reconnect
    ASSERT_TRUE(db.connect(conninfo));
    ASSERT_TRUE(db.is_open());
}

TEST(postgresql, set_encoding)
{
    if(!_is_pg_server_running())
        GTEST_SKIP()
            << "PostgreSQL server is not running or connection failed.";
    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());
    db.set_encoding("UTF8");
}

TEST(postgresql, exec_and_query)
{
    if(!_is_pg_server_running())
        GTEST_SKIP()
            << "PostgreSQL server is not running or connection failed.";

    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());

    db.exec("DROP TABLE IF EXISTS test_table");
    db.exec("CREATE TABLE test_table(id SERIAL PRIMARY KEY, name TEXT)");
    db.exec("INSERT INTO test_table(name) VALUES('Alice'),('Bob')");

    auto rows = db.query("SELECT name FROM test_table ORDER BY id");
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0][0], "Alice");
    EXPECT_EQ(rows[1][0], "Bob");

    db.exec("DROP TABLE test_table");
}

TEST(postgresql, batch_insert_and_empty_query)
{
    if(!_is_pg_server_running())
        GTEST_SKIP()
            << "PostgreSQL server is not running or connection failed.";
    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());
    db.exec("DROP TABLE IF EXISTS batch_table");
    db.exec("CREATE TABLE batch_table(id INT, val TEXT)");
    for(int i = 0; i < 10; ++i)
        ASSERT_TRUE(db.exec("INSERT INTO batch_table(id, val) VALUES("
                            + std::to_string(i) + ", 'v" + std::to_string(i)
                            + "')"));
    auto rows = db.query("SELECT val FROM batch_table ORDER BY id");
    ASSERT_EQ(rows.size(), 10u);
    for(int i = 0; i < 10; ++i)
        EXPECT_EQ(rows[i][0], "v" + std::to_string(i));
    db.exec("DROP TABLE batch_table");

    db.exec("CREATE TABLE batch_table(id INT, val TEXT)");
    auto empty = db.query("SELECT * FROM batch_table");
    ASSERT_EQ(empty.size(), 0u);
    db.exec("DROP TABLE batch_table");
}

TEST(postgresql, invalid_sql_and_reconnect)
{
    if(!_is_pg_server_running())
        GTEST_SKIP()
            << "PostgreSQL server is not running or connection failed.";
    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());

    ASSERT_FALSE(db.exec("SELECT * FROM not_exist_table"));

    db.disconnect();
    ASSERT_FALSE(db.is_open());
    ASSERT_FALSE(db.exec("SELECT 1"));

    ASSERT_TRUE(db.connect(conninfo));
    ASSERT_TRUE(db.is_open());
    ASSERT_TRUE(db.exec("SELECT 1"));
}