#include <gtest/gtest.h>
#include <libcpp/db/sqlite.hpp>
#include <cstdio>
#include <string>

using namespace libcpp;

bool _is_sqlite_valid()
{
    try {
        sqlite db;
        if (!db.open("test_check.db"))
            return false;
        db.close();
        remove("test_check.db");
        return true;
    } catch (...) {
        return false;
    }
}

int _sqlite_exec_cb(void* in, int argc, char** argv, char** col_name)
{
    auto* rows = static_cast<std::vector<std::vector<std::string>>*>(in);
    std::vector<std::string> row;
    for (int i = 0; i < argc; ++i)
        row.push_back(argv[i] ? argv[i] : "");
    rows->push_back(row);
    return 0;
}

class SqliteTest : public ::testing::Test {
protected:
	std::string dbfile = "test.db";
	void TearDown() override {
		remove(dbfile.c_str());
	}
};

TEST_F(SqliteTest, OpenClose) {
	if (!_is_sqlite_valid()) 
		GTEST_SKIP() << "sqlite not available";

	sqlite db;
	EXPECT_TRUE(db.open(dbfile));
	EXPECT_TRUE(db.is_open());
	db.close();
	EXPECT_FALSE(db.is_open());
}

TEST_F(SqliteTest, ExecAndQuery) {
	if (!_is_sqlite_valid()) 
		GTEST_SKIP() << "sqlite not available";

	sqlite db;
	ASSERT_TRUE(db.open(dbfile));
	EXPECT_TRUE(db.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, name TEXT);") );
	EXPECT_TRUE(db.exec("INSERT INTO t (name) VALUES ('Alice'),('Bob');"));
	auto rows = db.query("SELECT id, name FROM t ORDER BY id;", _sqlite_exec_cb);
	ASSERT_EQ(rows.size(), 2u);
	EXPECT_EQ(rows[0][1], "Alice");
	EXPECT_EQ(rows[1][1], "Bob");
}

TEST_F(SqliteTest, QueryEmpty) {
	if (!_is_sqlite_valid()) 
		GTEST_SKIP() << "sqlite not available";
		
	sqlite db;
	ASSERT_TRUE(db.open(dbfile));
	db.exec("CREATE TABLE t (id INTEGER);");
	auto rows = db.query("SELECT * FROM t;", _sqlite_exec_cb);
	EXPECT_TRUE(rows.empty());
}