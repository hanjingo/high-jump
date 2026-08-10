#include <gtest/gtest.h>
#include <hj/db/db_conn.hpp>
#include <sqlite3.h>

#include <cstdio>
#include <string>
#include <vector>
#include <future>
#include <utility>
#include <system_error>

using namespace hj;

class raw_sqlite_conn : public db_conn
{
  public:
    raw_sqlite_conn() = default;
    ~raw_sqlite_conn() override { close(); }

    bool open(const char *filename)
    {
        close();
        return sqlite3_open(filename, &_db) == SQLITE_OK;
    }

    void close()
    {
        if(_db)
        {
            sqlite3_close(_db);
            _db = nullptr;
        }
    }

    bool is_open() const { return _db != nullptr; }

    std::string id() const override { return "sqlite3_raw"; }

    err_t begin() override { return exec_simple("BEGIN TRANSACTION;"); }

    err_t commit() override { return exec_simple("COMMIT;"); }

    err_t rollback() override { return exec_simple("ROLLBACK;"); }

  protected:
    exec_result exec_impl(std::string_view sql,
                          const val_in_t  *args,
                          std::size_t      count) override
    {
        exec_result res{};
        if(!_db)
        {
            res.ec = std::make_error_code(std::errc::not_connected);
            return res;
        }

        sqlite3_stmt *stmt = nullptr;
        int           rc   = sqlite3_prepare_v2(_db,
                                                sql.data(),
                                                static_cast<int>(sql.size()),
                                                &stmt,
                                                nullptr);
        if(rc != SQLITE_OK)
        {
            res.ec = std::make_error_code(std::errc::invalid_argument);
            return res;
        }

        bind_params(stmt, args, count);

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if(rc == SQLITE_DONE || rc == SQLITE_ROW)
        {
            res.affected_rows  = sqlite3_changes(_db);
            res.last_insert_id = sqlite3_last_insert_rowid(_db);
            res.ec             = err_t{};
        } else
        {
            res.ec = std::make_error_code(std::errc::io_error);
        }
        return res;
    }

    void exec_async_impl(exec_cb_t              cb,
                         std::string            sql,
                         std::vector<out_val_t> args) override
    {
        auto        in_args = to_in_args(args);
        exec_result res     = exec_impl(sql, in_args.data(), in_args.size());
        if(cb)
            cb(res);
    }

    err_t query_impl(ret_t           &outs,
                     std::string_view sql,
                     const val_in_t  *args,
                     std::size_t      count) override
    {
        if(!_db)
            return std::make_error_code(std::errc::not_connected);

        sqlite3_stmt *stmt = nullptr;
        int           rc   = sqlite3_prepare_v2(_db,
                                                sql.data(),
                                                static_cast<int>(sql.size()),
                                                &stmt,
                                                nullptr);
        if(rc != SQLITE_OK)
            return std::make_error_code(std::errc::invalid_argument);

        bind_params(stmt, args, count);

        std::size_t            num_rows = 0;
        std::size_t            num_cols = 0;
        std::vector<out_val_t> fetched;

        while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            if(num_rows == 0)
            {
                num_cols = static_cast<std::size_t>(sqlite3_column_count(stmt));
            }
            num_rows++;
            for(std::size_t c = 0; c < num_cols; ++c)
            {
                int col_idx  = static_cast<int>(c);
                int col_type = sqlite3_column_type(stmt, col_idx);
                switch(col_type)
                {
                    case SQLITE_INTEGER:
                        fetched.emplace_back(static_cast<int64_t>(
                            sqlite3_column_int64(stmt, col_idx)));
                        break;
                    case SQLITE_FLOAT:
                        fetched.emplace_back(
                            sqlite3_column_double(stmt, col_idx));
                        break;
                    case SQLITE_TEXT: {
                        const char *txt = reinterpret_cast<const char *>(
                            sqlite3_column_text(stmt, col_idx));
                        int len = sqlite3_column_bytes(stmt, col_idx);
                        fetched.emplace_back(std::string(txt ? txt : "", len));
                        break;
                    }
                    case SQLITE_BLOB: {
                        const uint8_t *ptr = static_cast<const uint8_t *>(
                            sqlite3_column_blob(stmt, col_idx));
                        int len = sqlite3_column_bytes(stmt, col_idx);
                        fetched.emplace_back(blob_t(ptr, ptr + len));
                        break;
                    }
                    default:
                        fetched.emplace_back(nullptr);
                        break;
                }
            }
        }

        sqlite3_finalize(stmt);

        if(rc != SQLITE_DONE)
        {
            return std::make_error_code(std::errc::io_error);
        }

        outs.set_dim(num_rows, num_cols);
        outs.reserve(fetched.size());
        for(auto &v : fetched)
        {
            outs.emplace_back(std::move(v));
        }

        return err_t{};
    }

    void query_async_impl(query_cb_t             cb,
                          std::string            sql,
                          std::vector<out_val_t> args) override
    {
        auto  in_args = to_in_args(args);
        ret_t outs;
        err_t err = query_impl(outs, sql, in_args.data(), in_args.size());
        if(cb)
            cb(err, std::move(outs));
    }

  private:
    err_t exec_simple(const char *sql)
    {
        if(!_db)
            return std::make_error_code(std::errc::not_connected);
        char *err_msg = nullptr;
        int   rc      = sqlite3_exec(_db, sql, nullptr, nullptr, &err_msg);
        if(err_msg)
            sqlite3_free(err_msg);
        return (rc == SQLITE_OK) ? err_t{}
                                 : std::make_error_code(std::errc::io_error);
    }

    void
    bind_params(sqlite3_stmt *stmt, const val_in_t *args, std::size_t count)
    {
        for(std::size_t i = 0; i < count; ++i)
        {
            int idx = static_cast<int>(i + 1);
            std::visit(
                [stmt, idx](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<T, std::nullptr_t>)
                    {
                        sqlite3_bind_null(stmt, idx);
                    } else if constexpr(std::is_same_v<T, int64_t>)
                    {
                        sqlite3_bind_int64(stmt, idx, arg);
                    } else if constexpr(std::is_same_v<T, double>)
                    {
                        sqlite3_bind_double(stmt, idx, arg);
                    } else if constexpr(std::is_same_v<T, std::string_view>)
                    {
                        sqlite3_bind_text(stmt,
                                          idx,
                                          arg.data(),
                                          static_cast<int>(arg.size()),
                                          SQLITE_TRANSIENT);
                    } else if constexpr(std::is_same_v<T, blob_view>)
                    {
                        sqlite3_bind_blob(stmt,
                                          idx,
                                          arg.data,
                                          static_cast<int>(arg.size),
                                          SQLITE_TRANSIENT);
                    }
                },
                args[i]);
        }
    }

    std::vector<val_in_t> to_in_args(const std::vector<out_val_t> &args)
    {
        std::vector<val_in_t> in_args;
        in_args.reserve(args.size());
        for(const auto &v : args)
        {
            std::visit(
                [&in_args](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<T, std::nullptr_t>)
                        in_args.push_back(nullptr);
                    else if constexpr(std::is_same_v<T, int64_t>)
                        in_args.push_back(arg);
                    else if constexpr(std::is_same_v<T, double>)
                        in_args.push_back(arg);
                    else if constexpr(std::is_same_v<T, std::string>)
                        in_args.push_back(std::string_view(arg));
                    else if constexpr(std::is_same_v<T, blob_t>)
                        in_args.push_back(blob_view{arg.data(), arg.size()});
                },
                v);
        }
        return in_args;
    }

    sqlite3 *_db{nullptr};
};

static bool _is_sqlite_conn_valid()
{
    sqlite3 *db = nullptr;
    if(sqlite3_open("test_conn_check.db", &db) != SQLITE_OK)
        return false;

    sqlite3_close(db);
    std::remove("test_conn_check.db");
    return true;
}

TEST(db_conn, exec_and_query_types)
{
    if(!_is_sqlite_conn_valid())
        GTEST_SKIP() << "sqlite not available";

    raw_sqlite_conn db;
    ASSERT_TRUE(db.open("ConnTypesTest.db"));
    db_conn &conn = db;

    conn.exec("DROP TABLE IF EXISTS t_types;");
    auto create_res = conn.exec("CREATE TABLE t_types ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                "i_val INTEGER, "
                                "f_val REAL, "
                                "s_val TEXT, "
                                "b_val BLOB, "
                                "n_val TEXT);");
    EXPECT_FALSE(create_res.ec);

    int64_t            i_in       = 987654321012345LL;
    double             f_in       = 3.14159265;
    std::string_view   s_in       = "hello db_conn";
    uint8_t            raw_blob[] = {0x12, 0x34, 0x56, 0x00, 0x78, 0x9A};
    db_conn::blob_view b_in{raw_blob, sizeof(raw_blob)};

    auto ins_res =
        conn.exec("INSERT INTO t_types (i_val, f_val, s_val, b_val, n_val) "
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
    auto           query_err = conn.query(
        outs,
        "SELECT i_val, f_val, s_val, b_val, n_val FROM t_types WHERE id = ?;",
        ins_res.last_insert_id);

    EXPECT_FALSE(query_err);
    ASSERT_EQ(outs.rows(), 1u);
    ASSERT_EQ(outs.cols(), 5u);

    EXPECT_EQ(outs.get_or<int64_t>(0, 0, 0), i_in);
    EXPECT_NEAR(outs.get_or<double>(0, 1, 0.0), f_in, 1e-6);
    EXPECT_EQ(outs.get_or<std::string>(0, 2, ""), "hello db_conn");

    const auto *p_blob = outs.get_if<db_conn::blob_t>(0, 3);
    ASSERT_NE(p_blob, nullptr);
    db_conn::blob_t expected_blob(raw_blob, raw_blob + sizeof(raw_blob));
    EXPECT_EQ(*p_blob, expected_blob);

    EXPECT_TRUE(outs.is_null(0, 4));

    auto row0 = outs[0];
    EXPECT_EQ(std::get<int64_t>(row0[0]), i_in);
    EXPECT_EQ(std::get<std::string>(row0[2]), "hello db_conn");

    db.close();
    std::remove("ConnTypesTest.db");
}

TEST(db_conn, dynamic_array_binding)
{
    if(!_is_sqlite_conn_valid())
        GTEST_SKIP() << "sqlite not available";

    raw_sqlite_conn db;
    ASSERT_TRUE(db.open("ConnDynTest.db"));
    db_conn &conn = db;

    conn.exec("DROP TABLE IF EXISTS t_dyn;");
    conn.exec("CREATE TABLE t_dyn (id INTEGER, name TEXT);");

    std::vector<db_conn::val_in_t> args;
    args.push_back(int64_t(1001));
    args.push_back(std::string_view("dynamic_user"));

    auto res = conn.exec("INSERT INTO t_dyn (id, name) VALUES (?, ?);",
                         args.data(),
                         args.size());
    EXPECT_FALSE(res.ec);
    EXPECT_EQ(res.affected_rows, 1);

    db_conn::ret_t outs;
    auto           err = conn.query(outs,
                                    "SELECT name FROM t_dyn WHERE id = ?;",
                                    args.data(),
                                    1);
    EXPECT_FALSE(err);
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "dynamic_user");

    db.close();
    std::remove("ConnDynTest.db");
}

TEST(db_conn, transaction)
{
    if(!_is_sqlite_conn_valid())
        GTEST_SKIP() << "sqlite not available";

    raw_sqlite_conn db;
    ASSERT_TRUE(db.open("ConnTransTest.db"));
    db_conn &conn = db;

    conn.exec("DROP TABLE IF EXISTS t_trans;");
    conn.exec("CREATE TABLE t_trans (id INTEGER PRIMARY KEY, val TEXT);");

    EXPECT_FALSE(conn.begin());
    conn.exec("INSERT INTO t_trans (val) VALUES ('rollback_me');");
    EXPECT_FALSE(conn.rollback());

    db_conn::ret_t outs;
    conn.query(outs, "SELECT * FROM t_trans;");
    EXPECT_TRUE(outs.empty());

    EXPECT_FALSE(conn.begin());
    conn.exec("INSERT INTO t_trans (val) VALUES ('commit_me');");
    EXPECT_FALSE(conn.commit());

    conn.query(outs, "SELECT val FROM t_trans;");
    ASSERT_EQ(outs.rows(), 1u);
    EXPECT_EQ(outs.get_or<std::string>(0, 0, ""), "commit_me");

    db.close();
    std::remove("ConnTransTest.db");
}

TEST(db_conn, async_operations)
{
    if(!_is_sqlite_conn_valid())
        GTEST_SKIP() << "sqlite not available";

    raw_sqlite_conn db;
    ASSERT_TRUE(db.open("ConnAsyncTest.db"));
    db_conn &conn = db;

    conn.exec("DROP TABLE IF EXISTS t_async;");
    conn.exec("CREATE TABLE t_async (id INTEGER PRIMARY KEY, val TEXT);");

    std::promise<db_conn::exec_result> exec_prom;
    auto                               exec_fut = exec_prom.get_future();

    conn.exec_async(
        [&exec_prom](db_conn::exec_result res) { exec_prom.set_value(res); },
        "INSERT INTO t_async (id, val) VALUES (?, ?);",
        200LL,
        "async_value");

    auto exec_res = exec_fut.get();
    EXPECT_FALSE(exec_res.ec);
    EXPECT_EQ(exec_res.affected_rows, 1);

    std::promise<std::pair<db_conn::err_t, db_conn::ret_t>> query_prom;
    auto query_fut = query_prom.get_future();

    conn.query_async(
        [&query_prom](db_conn::err_t ec, db_conn::ret_t outs) {
            query_prom.set_value({ec, std::move(outs)});
        },
        "SELECT val FROM t_async WHERE id = ?;",
        200LL);

    auto [q_err, q_outs] = query_fut.get();
    EXPECT_FALSE(q_err);
    ASSERT_EQ(q_outs.rows(), 1u);
    EXPECT_EQ(q_outs.get_or<std::string>(0, 0, ""), "async_value");

    db.close();
    std::remove("ConnAsyncTest.db");
}

TEST(db_conn, error_handling)
{
    if(!_is_sqlite_conn_valid())
        GTEST_SKIP() << "sqlite not available";

    raw_sqlite_conn db;
    ASSERT_TRUE(db.open("ConnErrTest.db"));
    db_conn &conn = db;

    auto res = conn.exec("INVALID SQL SYNTAX STATEMENT;");
    EXPECT_TRUE(res.ec);

    db_conn::ret_t outs;
    auto           err = conn.query(outs, "SELECT * FROM non_existent_table;");
    EXPECT_TRUE(err);
    EXPECT_TRUE(outs.empty());

    db.close();
    std::remove("ConnErrTest.db");
}