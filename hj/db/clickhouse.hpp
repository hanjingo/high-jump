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

#ifndef CLICKHOUSE_HPP
#define CLICKHOUSE_HPP

#if CLICKHOUSE_ENABLE

#include <clickhouse/client.h>

namespace hj
{

namespace ck
{
using client_options = clickhouse::ClientOptions;
using block          = clickhouse::Block;

using column_uint8     = clickhouse::ColumnUInt8;
using column_int8      = clickhouse::ColumnInt8;
using column_uint16    = clickhouse::ColumnUInt16;
using column_int16     = clickhouse::ColumnInt16;
using column_uint32    = clickhouse::ColumnUInt32;
using column_int32     = clickhouse::ColumnInt32;
using column_uint64    = clickhouse::ColumnUInt64;
using column_int64     = clickhouse::ColumnInt64;
using column_float32   = clickhouse::ColumnFloat32;
using column_float64   = clickhouse::ColumnFloat64;
using column_string    = clickhouse::ColumnString;
using column_date_time = clickhouse::ColumnDateTime;
using column_array     = clickhouse::ColumnArray;
using column_decimal   = clickhouse::ColumnDecimal;

class client
{
  public:
    explicit client(const client_options &opt)
        : _client{std::make_unique<clickhouse::Client>(opt)}
    {
    }

    client(const std::string &host,
           const uint16_t     port,
           const std::string &user,
           const std::string &password,
           const std::string &db)
    {
        client_options opts;
        opts.SetHost(host);
        opts.SetPort(port);
        opts.SetUser(user);
        opts.SetPassword(password);
        opts.SetDefaultDatabase(db);
        _client = std::make_unique<clickhouse::Client>(opts);
    }

    ~client() = default;

    inline bool is_connected() const noexcept
    {
        try
        {
            return _client->GetCurrentEndpoint().has_value();
        }
        catch(...)
        {
            return false;
        }
    }

    inline void execute(const std::string &sql) noexcept
    {
        try
        {
            _client->Execute(clickhouse::Query(sql));
        }
        catch(...)
        {
        }
    }

    inline std::vector<block> select(const std::string &sql) noexcept
    {
        std::vector<block> results;
        try
        {
            _client->Select(sql, [&](const block &b) { results.push_back(b); });
        }
        catch(...)
        {
        }
        return results;
    }

    inline void insert(const std::string &table, const block &b) noexcept
    {
        try
        {
            _client->Insert(table, b);
        }
        catch(...)
        {
        }
    }

  private:
    std::unique_ptr<clickhouse::Client> _client;
};

static void append_column(block                       &b,
                          const std::string           &name,
                          const clickhouse::ColumnRef &col)
{
    b.AppendColumn(name, col);
}

} // namespace ck
} // namespace hj

#endif // CLICKHOUSE_ENABLE

#endif