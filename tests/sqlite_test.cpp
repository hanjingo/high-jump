#include <gtest/gtest.h>
#include <hj/db/sqlite.hpp>
#include <cstdio>
#include <string>

using namespace hj;

bool _is_sqlite_valid()
{
    try
    {
        sqlite db;
        if(!db.open("test_check.db"))
            return false;

        db.close();
        remove("test_check.db");
        return true;
    }
    catch(...)
    {
        return false;
    }
}

int _sqlite_exec_cb(void *in, int argc, char **argv, char **col_name)
{
    auto *rows = static_cast<std::vector<std::vector<std::string>> *>(in);
    std::vector<std::string> row;
    for(int i = 0; i < argc; ++i)
        row.push_back(argv[i] ? argv[i] : "");
    rows->push_back(row);
    return 0;
}

TEST(sqlite, open_close)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    EXPECT_TRUE(db.open("OpenCloseTest.db"));
    EXPECT_TRUE(db.is_open());
    db.close();
    EXPECT_FALSE(db.is_open());
    remove("OpenCloseTest.db");
}

TEST(sqlite, exec_and_query)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("ExecAndQueryTest.db"));
    EXPECT_TRUE(db.exec("DROP TABLE IF EXISTS t;"));
    EXPECT_TRUE(db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, name TEXT);"));
    EXPECT_TRUE(db.exec("INSERT INTO t (name) VALUES ('Alice'),('Bob');"));
    auto rows =
        db.query("SELECT id, name FROM t ORDER BY id;", _sqlite_exec_cb);
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0][1], "Alice");
    EXPECT_EQ(rows[1][1], "Bob");

    EXPECT_TRUE(db.exec("INSERT INTO t (name) VALUES ('C''arol');"));
    rows = db.query("SELECT id, name FROM t WHERE name='C''arol';",
                    _sqlite_exec_cb);
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0][1], "C'arol");

    EXPECT_TRUE(db.exec("INSERT INTO t (name) VALUES (''),('');"));
    rows = db.query("SELECT name FROM t WHERE name='';", _sqlite_exec_cb);
    ASSERT_EQ(rows.size(), 2u);
    db.close();
    remove("ExecAndQueryTest.db");
}
TEST(sqlite, transaction)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("TransTest.db"));
    EXPECT_TRUE(db.exec("DROP TABLE IF EXISTS t;"));
    EXPECT_TRUE(db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT);"));
    EXPECT_TRUE(db.begin());
    EXPECT_TRUE(db.exec("INSERT INTO t (v) VALUES ('x');"));
    EXPECT_TRUE(db.rollback());
    auto rows = db.query("SELECT * FROM t;", _sqlite_exec_cb);
    EXPECT_TRUE(rows.empty());
    EXPECT_TRUE(db.begin());
    EXPECT_TRUE(db.exec("INSERT INTO t (v) VALUES ('y');"));
    EXPECT_TRUE(db.commit());
    rows = db.query("SELECT v FROM t;", _sqlite_exec_cb);
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0][0], "y");
    db.close();
    remove("TransTest.db");
}

TEST(sqlite, exec_error)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";
    sqlite db;
    ASSERT_TRUE(db.open("ExecErrTest.db"));
    EXPECT_FALSE(db.exec("INVALID SQL;"));
    std::string err = db.get_last_error();
    EXPECT_FALSE(err.empty());
    db.close();
    remove("ExecErrTest.db");
}

TEST(sqlite, query_no_table)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";
    sqlite db;
    ASSERT_TRUE(db.open("NoTableTest.db"));
    auto rows = db.query("SELECT * FROM not_exist;", _sqlite_exec_cb);
    EXPECT_TRUE(rows.empty());
    db.close();
    remove("NoTableTest.db");
}

TEST(sqlite, open_close_many)
{
    for(int i = 0; i < 10; ++i)
    {
        sqlite db;
        EXPECT_TRUE(db.open("OpenCloseManyTest.db"));
        EXPECT_TRUE(db.is_open());
        db.close();
        EXPECT_FALSE(db.is_open());
    }
    remove("OpenCloseManyTest.db");
}

TEST(sqlite, query_empty)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("QueryEmptyTest.db"));
    EXPECT_TRUE(db.exec("DROP TABLE IF EXISTS t;"));
    db.exec("CREATE TABLE t (id INTEGER);");
    auto rows = db.query("SELECT * FROM t;", _sqlite_exec_cb);
    EXPECT_TRUE(rows.empty());
    db.close();
    remove("QueryEmptyTest.db");
}

TEST(sqlite, get_last_error)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("GetLastErrorTest.db"));
    EXPECT_FALSE(db.exec("SELECT * FROM not_exist_table;"));
    std::string err = db.get_last_error();
    EXPECT_FALSE(err.empty());
    EXPECT_NE(err.find("no such table"), std::string::npos);
    db.close();
    remove("GetLastErrorTest.db");
}