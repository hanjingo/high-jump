#include "sqlite.h"

int sqlite::exec(const char* sql)
{
    auto conn = _pool.acquire(100);

    if (!conn || !conn->is_open())
        return DB_ERR_SQLITE_GET_CONN_FAIL;

    if (!conn->exec(sql))
        return DB_OK;

    return DB_ERR_SQLITE_EXEC_FAIL;
}

sqlite::conn_ptr_t sqlite::_make_conn()
{
    auto conn = std::make_shared<conn_t>();
    conn->open(_path);
    return conn;
}

bool sqlite::_check_conn(sqlite::conn_ptr_t conn)
{
    return conn && conn->is_open();
}