#include <gtest/gtest.h>
#include <libcpp/db/postgresql.hpp>
#include <string>
#include <vector>

// NOTE: Requires a running PostgreSQL server and a test database/user.
// Example connection string: "host=localhost port=5432 dbname=testdb user=testuser password=testpass"

const std::string conninfo = "host=localhost port=5432 dbname=postgres user=postgres password=postgres";

TEST(PostgreSQLTest, ConnectAndDisconnect) {
    libcpp::postgresql db(conninfo);
    ASSERT_TRUE(db.is_open());
    db.disconnect();
    ASSERT_FALSE(db.is_open());
}

TEST(PostgreSQLTest, ExecAndQuery) {
    libcpp::postgresql db(conninfo);
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