/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SQLITE_HPP
#define SQLITE_HPP

#include "db_conn.hpp"

#include <sqlite3.h>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace hj
{

class sqlite : public db_conn
{
  public:
    sqlite() = default;

    explicit sqlite(const std::string &filename) { open(filename); }

    ~sqlite() override { close(); }

    sqlite(sqlite &&other) noexcept
        : _db(std::exchange(other._db, nullptr))
        , _db_path(std::move(other._db_path))
        , _last_err(std::move(other._last_err))
    {
    }

    sqlite &operator=(sqlite &&other) noexcept
    {
        if(this != &other)
        {
            close();
            _db       = std::exchange(other._db, nullptr);
            _db_path  = std::move(other._db_path);
            _last_err = std::move(other._last_err);
        }
        return *this;
    }

    static std::string mprintf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        char *result = sqlite3_vmprintf(format, args);
        va_end(args);
        if(result == nullptr)
            return "";

        std::string str(result);
        sqlite3_free(result);
        return str;
    }

    bool open(const std::string &filename)
    {
        close();
        _last_err.clear();
        if(sqlite3_open(filename.c_str(), &_db) == SQLITE_OK)
        {
            _db_path = filename;
            return true;
        }

        if(_db)
        {
            _last_err = sqlite3_errmsg(_db);
            sqlite3_close_v2(_db);
            _db = nullptr;
        } else
        {
            _last_err = "Failed to allocate sqlite memory";
        }
        return false;
    }

    void close()
    {
        if(!_db)
            return;

        sqlite3_close_v2(_db);
        _db = nullptr;
    }

    inline bool        is_open() const { return _db != nullptr; }
    inline std::string get_last_error() const { return _last_err; }
    inline std::string path() const { return _db_path; }

  protected:
    err_t begin_impl() override
    {
        auto res = exec("BEGIN TRANSACTION;");
        return res.ec;
    }

    err_t commit_impl() override
    {
        auto res = exec("COMMIT;");
        return res.ec;
    }

    err_t rollback_impl() override
    {
        auto res = exec("ROLLBACK;");
        return res.ec;
    }

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
            _last_err = sqlite3_errmsg(_db);
            res.ec    = std::make_error_code(std::errc::bad_message);
            return res;
        }

        if(!_bind_stmt_args(stmt, args, count))
        {
            res.ec = std::make_error_code(std::errc::invalid_argument);
            sqlite3_finalize(stmt);
            return res;
        }

        rc = sqlite3_step(stmt);
        if(rc == SQLITE_DONE || rc == SQLITE_ROW)
        {
            res.ec             = err_t();
            res.affected_rows  = sqlite3_changes(_db);
            res.last_insert_id = sqlite3_last_insert_rowid(_db);
        } else
        {
            _last_err = sqlite3_errmsg(_db);
            res.ec    = std::make_error_code(std::errc::io_error);
        }

        sqlite3_finalize(stmt);
        return res;
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
        {
            _last_err = sqlite3_errmsg(_db);
            return std::make_error_code(std::errc::bad_message);
        }

        if(!_bind_stmt_args(stmt, args, count))
        {
            sqlite3_finalize(stmt);
            return std::make_error_code(std::errc::invalid_argument);
        }

        int                    cols = sqlite3_column_count(stmt);
        std::vector<out_val_t> temp_data;
        std::size_t            rows_cnt = 0;

        while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            ++rows_cnt;
            for(int col = 0; col < cols; ++col)
            {
                int type = sqlite3_column_type(stmt, col);
                switch(type)
                {
                    case SQLITE_INTEGER:
                        temp_data.emplace_back(static_cast<int64_t>(
                            sqlite3_column_int64(stmt, col)));
                        break;
                    case SQLITE_FLOAT:
                        temp_data.emplace_back(
                            sqlite3_column_double(stmt, col));
                        break;
                    case SQLITE_TEXT: {
                        const char *txt = reinterpret_cast<const char *>(
                            sqlite3_column_text(stmt, col));
                        int bytes = sqlite3_column_bytes(stmt, col);
                        temp_data.emplace_back(std::string(txt, bytes));
                        break;
                    }
                    case SQLITE_BLOB: {
                        const uint8_t *blob_bytes =
                            reinterpret_cast<const uint8_t *>(
                                sqlite3_column_blob(stmt, col));
                        int bytes = sqlite3_column_bytes(stmt, col);
                        temp_data.emplace_back(
                            blob_t(blob_bytes, blob_bytes + bytes));
                        break;
                    }
                    case SQLITE_NULL:
                    default:
                        temp_data.emplace_back(nullptr);
                        break;
                }
            }
        }

        sqlite3_finalize(stmt);
        if(rc != SQLITE_DONE)
        {
            _last_err = sqlite3_errmsg(_db);
            return std::make_error_code(std::errc::io_error);
        }

        outs.set_dim(rows_cnt, static_cast<std::size_t>(cols));
        outs.reserve(temp_data.size());
        for(auto &&val : temp_data)
            outs.emplace_back(std::move(val));

        return err_t();
    }

  private:
    sqlite(const sqlite &)            = delete;
    sqlite &operator=(const sqlite &) = delete;

    bool
    _bind_stmt_args(sqlite3_stmt *stmt, const val_in_t *args, std::size_t count)
    {
        if(!args || count == 0)
            return true;

        for(std::size_t i = 0; i < count; ++i)
        {
            int idx = static_cast<int>(i + 1);
            int rc  = SQLITE_OK;
            std::visit(
                [stmt, idx, &rc](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<T, std::nullptr_t>)
                    {
                        rc = sqlite3_bind_null(stmt, idx);
                    } else if constexpr(std::is_same_v<T, int64_t>)
                    {
                        rc = sqlite3_bind_int64(stmt, idx, arg);
                    } else if constexpr(std::is_same_v<T, double>)
                    {
                        rc = sqlite3_bind_double(stmt, idx, arg);
                    } else if constexpr(std::is_same_v<T, std::string_view>)
                    {
                        rc = sqlite3_bind_text(stmt,
                                               idx,
                                               arg.data(),
                                               static_cast<int>(arg.size()),
                                               SQLITE_TRANSIENT);
                    } else if constexpr(std::is_same_v<T, blob_view>)
                    {
                        rc = sqlite3_bind_blob(stmt,
                                               idx,
                                               arg.data,
                                               static_cast<int>(arg.size),
                                               SQLITE_TRANSIENT);
                    }
                },
                args[i]);

            if(rc != SQLITE_OK)
            {
                _last_err = sqlite3_errmsg(_db);
                return false;
            }
        }
        return true;
    }

  private:
    sqlite3    *_db{nullptr};
    std::string _db_path;
    std::string _last_err;
};

} // namespace hj

#endif // SQLITE_HPP