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
    ~client() { _client.release(); }

    inline bool is_connected() const
    {
        return _client->GetCurrentEndpoint().has_value();
    }

    inline void execute(const std::string &sql)
    {
        _client->Execute(clickhouse::Query(sql));
    }

    inline block select(const std::string &sql)
    {
        block result;
        _client->Select(sql, [&](const block &b) { result = b; });
        return result;
    }

    inline void insert(const std::string &table, const block &b)
    {
        _client->Insert(table, b);
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