#include <gtest/gtest.h>
#include <hj/db/sqlite.hpp>

#include <cstdio>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace hj;

static bool _is_sqlite_valid()
{
    sqlite db;
    if(!db.open("test_check.db"))
        return false;

    db.close();
    std::remove("test_check.db");
    return true;
}

TEST(sqlite, mprintf)
{
    std::string str;
    str = sqlite::mprintf("Hello %s, %d", "World", 123);
    EXPECT_EQ(str, "Hello World, 123");

    str = sqlite::mprintf("No args");
    EXPECT_EQ(str, "No args");

    str = sqlite::mprintf("Percent %% sign");
    EXPECT_EQ(str, "Percent % sign");

    str = sqlite::mprintf("Hello %q, %d, %lld", "World", 123, 456789LL);
    EXPECT_EQ(str, "Hello World, 123, 456789");

    str = sqlite::mprintf("Null string: %q", nullptr);
    EXPECT_EQ(str, "Null string: (NULL)");

    str = sqlite::mprintf("Hello %Q, %d, %lld", "World", 123, 456789LL);
    EXPECT_EQ(str, "Hello 'World', 123, 456789");

    str = sqlite::mprintf("Empty string: %Q", "");
    EXPECT_EQ(str, "Empty string: ''");
}

TEST(sqlite, open_close_and_path)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    EXPECT_FALSE(db.is_open());

    EXPECT_TRUE(db.open("OpenCloseTest.db"));
    EXPECT_TRUE(db.is_open());
    EXPECT_EQ(db.path(), "OpenCloseTest.db");

    db.close();
    EXPECT_FALSE(db.is_open());

    std::remove("OpenCloseTest.db");
}

TEST(sqlite, move_semantics)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db1("MoveTest.db");
    ASSERT_TRUE(db1.is_open());

    sqlite db2(std::move(db1));
    EXPECT_FALSE(db1.is_open());
    EXPECT_TRUE(db2.is_open());
    EXPECT_EQ(db2.path(), "MoveTest.db");

    sqlite db3;
    db3 = std::move(db2);
    EXPECT_FALSE(db2.is_open());
    EXPECT_TRUE(db3.is_open());
    EXPECT_EQ(db3.path(), "MoveTest.db");

    db3.close();
    std::remove("MoveTest.db");
}

TEST(sqlite, exec_and_query_types)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("ExecQueryTypesTest.db"));

    auto drop_res = db.exec("DROP TABLE IF EXISTS t_all_types;");
    EXPECT_FALSE(drop_res.ec);

    auto create_res = db.exec("CREATE TABLE t_all_types ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "i_val INTEGER, "
                              "f_val REAL, "
                              "s_val TEXT, "
                              "b_val BLOB, "
                              "n_val TEXT);");
    EXPECT_FALSE(create_res.ec);

    int64_t            i_in       = 123456789012345LL;
    double             f_in       = 2.718281828;
    std::string_view   s_in       = "sqlite_test_string";
    uint8_t            raw_blob[] = {0xAA, 0xBB, 0xCC, 0x00, 0xDD, 0xEE};
    db_conn::blob_view b_in{raw_blob, sizeof(raw_blob)};

    auto ins_res =
        db.exec("INSERT INTO t_all_types (i_val, f_val, s_val, b_val, n_val) "
                "VALUES (?, ?, ?, ?, ?);",
                i_in,
                f_in,
                s_in,
                b_in,
                nullptr);

    EXPECT_FALSE(ins_res.ec);
    EXPECT_EQ(ins_res.affected_rows, 1);
    EXPECT_GT(ins_res.last_insert_id, 0);

    db_conn::ret_t outs;
    auto query_err = db.query(outs,
                              "SELECT i_val, f_val, s_val, b_val, n_val "
                              "FROM t_all_types WHERE id = ?;",
                              ins_res.last_insert_id);

    EXPECT_FALSE(query_err);
    ASSERT_EQ(outs.rows(), 1u);
    ASSERT_EQ(outs.cols(), 5u);

    EXPECT_EQ(outs.get_or<int64_t>(0, 0, 0), i_in);
    EXPECT_NEAR(outs.get_or<double>(0, 1, 0.0), f_in, 1e-6);
    EXPECT_EQ(outs.get_or<std::string>(0, 2, ""), "sqlite_test_string");

    const auto *p_blob = outs.get_if<db_conn::blob_t>(0, 3);
    ASSERT_NE(p_blob, nullptr);
    db_conn::blob_t expected_blob(raw_blob, raw_blob + sizeof(raw_blob));
    EXPECT_EQ(*p_blob, expected_blob);

    EXPECT_TRUE(outs.is_null(0, 4));

    db.close();
    std::remove("ExecQueryTypesTest.db");
}

TEST(sqlite, transaction_manual)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("TransManualTest.db"));

    db.exec("DROP TABLE IF EXISTS t_trans;");
    db.exec("CREATE TABLE t_trans (id INTEGER PRIMARY KEY, v TEXT);");

    EXPECT_FALSE(db.begin());
    db.exec("INSERT INTO t_trans (v) VALUES ('rolled_back');");
    EXPECT_FALSE(db.rollback());

    db_conn::ret_t outs;
    db.query(outs, "SELECT * FROM t_trans;");
    EXPECT_TRUE(outs.empty());

    EXPECT_FALSE(db.begin());
    db.exec("INSERT INTO t_trans (v) VALUES ('committed');");
    EXPECT_FALSE(db.commit());

    db.query(outs, "SELECT v FROM t_trans;");
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "committed");

    db.close();
    std::remove("TransManualTest.db");
}

TEST(sqlite, transaction_guard)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("TransGuardTest.db"));

    db.exec("DROP TABLE IF EXISTS t_guard;");
    db.exec("CREATE TABLE t_guard (id INTEGER PRIMARY KEY, v TEXT);");

    {
        db_conn::trans_guard guard(db);
        db.exec("INSERT INTO t_guard (v) VALUES ('rollback_value');");
    }

    db_conn::ret_t outs;
    db.query(outs, "SELECT * FROM t_guard;");
    EXPECT_TRUE(outs.empty());

    {
        db_conn::trans_guard guard(db);
        db.exec("INSERT INTO t_guard (v) VALUES ('commit_value');");
        EXPECT_FALSE(guard.commit());
    }

    db.query(outs, "SELECT v FROM t_guard;");
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "commit_value");

    db.close();
    std::remove("TransGuardTest.db");
}

TEST(sqlite, error_handling)
{
    if(!_is_sqlite_valid())
        GTEST_SKIP() << "sqlite not available";

    sqlite db;
    ASSERT_TRUE(db.open("ExecErrTest.db"));

    auto res = db.exec("INVALID SQL STATEMENT;");
    EXPECT_TRUE(res.ec);
    std::string err_msg = db.get_last_error();
    EXPECT_FALSE(err_msg.empty());

    db_conn::ret_t outs;
    auto           q_err = db.query(outs, "SELECT * FROM non_existent_table;");
    EXPECT_TRUE(q_err);
    EXPECT_TRUE(outs.empty());
    EXPECT_FALSE(db.get_last_error().empty());

    db.close();
    std::remove("ExecErrTest.db");
}

TEST(sqlite, open_close_many)
{
    for(int i = 0; i < 5; ++i)
    {
        sqlite db;
        EXPECT_TRUE(db.open("OpenCloseManyTest.db"));
        EXPECT_TRUE(db.is_open());
        db.close();
        EXPECT_FALSE(db.is_open());
    }
    std::remove("OpenCloseManyTest.db");
}