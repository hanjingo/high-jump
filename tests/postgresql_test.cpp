#include <gtest/gtest.h>
#include <hj/db/postgresql.hpp>
#include <string>
#include <vector>

// NOTE: Requires a running PostgreSQL server and a test database/user.
// Example connection string: "host=localhost port=5432 dbname=testdb user=testuser password=testpass"

const std::string conninfo = "host=localhost port=5432 dbname=postgres user=postgres password=postgres";

static bool _is_pg_server_running()
{
    // Check if PostgreSQL server is running
    try
    {
        hj::pg_connection conn("dbname=postgres user=postgres password=postgres");
        if (conn.is_open())
        {
            conn.disconnect();
            return true;
        }
    }
    catch (const std::exception& e)
    {
        // Handle exception (e.g., log it)
    }
    return false;
}

TEST(PostgreSQLTest, ConnectAndDisconnect) 
{
    if (!_is_pg_server_running())
        GTEST_SKIP() << "PostgreSQL server is not running or connection failed.";

    hj::pg_connection db(conninfo);
    ASSERT_TRUE(db.is_open());
    db.disconnect();
    ASSERT_FALSE(db.is_open());
}

TEST(PostgreSQLTest, ExecAndQuery) 
{
    if (!_is_pg_server_running())
        GTEST_SKIP() << "PostgreSQL server is not running or connection failed.";

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