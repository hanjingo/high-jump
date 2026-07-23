#ifndef DB_HPP
#define DB_HPP

#ifdef HJ_ENABLE_CK
#include <hj/db/clickhouse.hpp>
#endif

#include <hj/db/db_conn_pool.hpp>

#ifdef HJ_ENABLE_REDIS
#include <hj/db/redis.hpp>
#endif

#ifdef HJ_ENABLE_SQLITE
#include <hj/db/sqlite.hpp>
#endif

#endif