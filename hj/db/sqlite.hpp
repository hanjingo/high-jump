/*
 *  This file is part of hj.
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
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

#include <sqlite3.h>
#include <string>
#include <vector>
#include <stdexcept>

namespace hj
{

class sqlite
{
  public:
    typedef int (*exec_cb)(void *in, int argc, char **argv, char **col_name);

  public:
    sqlite()
        : _db{nullptr}
    {
    }
    ~sqlite() { close(); }

    bool open(const std::string &filename)
    {
        close();
        _last_err.clear();
        if(sqlite3_open(filename.c_str(), &_db) == SQLITE_OK)
            return true;

        _last_err = sqlite3_errmsg(_db ? _db : nullptr);
        close();
        return false;
    }

    void close()
    {
        if(!_db)
            return;

        sqlite3_close(_db);
        _db = nullptr;
    }

    inline bool        is_open() const { return _db != nullptr; }
    inline std::string get_last_error() { return _last_err; }
    inline bool        begin() { return exec("BEGIN TRANSACTION;"); }
    inline bool        commit() { return exec("COMMIT;"); }
    inline bool        rollback() { return exec("ROLLBACK;"); }

    bool exec(const std::string &sql)
    {
        if(!_db)
            return false;

        char *errmsg = nullptr;
        if(sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errmsg)
           == SQLITE_OK)
            return true;

        if(errmsg)
        {
            _last_err.assign(errmsg, strlen(errmsg));
            sqlite3_free(errmsg);
        } else
        {
            _last_err = sqlite3_errmsg(_db);
        }
        return false;
    }

    // Query: returns vector of rows, each row is vector<string>
    std::vector<std::vector<std::string>> query(const std::string &sql,
                                                exec_cb            callback)
    {
        std::vector<std::vector<std::string>> rows;
        if(!_db)
            return rows;

        sqlite3_stmt *stmt = nullptr;
        if(sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr)
           != SQLITE_OK)
        {
            _last_err = sqlite3_errmsg(_db);
            return rows;
        }

        int cols = sqlite3_column_count(stmt);
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::vector<std::string> row;
            for(int i = 0; i < cols; ++i)
            {
                const char *val = reinterpret_cast<const char *>(
                    sqlite3_column_text(stmt, i));
                row.push_back(val ? val : "");
            }
            rows.push_back(std::move(row));
        }
        sqlite3_finalize(stmt);
        return rows;
    }

  private:
    sqlite(const sqlite &)            = delete;
    sqlite &operator=(const sqlite &) = delete;
    sqlite(sqlite &&)                 = delete;
    sqlite &operator=(sqlite &&)      = delete;

  private:
    sqlite3    *_db = nullptr;
    std::string _last_err;
};

} // namespace hj

#endif