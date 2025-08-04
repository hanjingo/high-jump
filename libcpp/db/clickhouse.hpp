#ifndef CLICKHOUSE_HPP
#define CLICKHOUSE_HPP

#include <clickhouse/client.h>

namespace libcpp
{

namespace ck
{

using client = clickhouse::Client;
using client_options = clickhouse::ClientOptions;
using block = clickhouse::Block;
using query = clickhouse::Query;

using column_uint8 = clickhouse::ColumnUInt8;
using column_int8 = clickhouse::ColumnInt8;
using column_uint16 = clickhouse::ColumnUInt16;
using column_int16 = clickhouse::ColumnInt16;
using column_uint32 = clickhouse::ColumnUInt32;
using column_int32 = clickhouse::ColumnInt32;
using column_uint64 = clickhouse::ColumnUInt64;
using column_int64 = clickhouse::ColumnInt64;
using column_float32 = clickhouse::ColumnFloat32;
using column_float64 = clickhouse::ColumnFloat64;
using column_string = clickhouse::ColumnString;
using column_date_time = clickhouse::ColumnDateTime;
using column_array = clickhouse::ColumnArray;
using column_decimal = clickhouse::ColumnDecimal;

} // namespace ck

} // namespace libcpp

#endif