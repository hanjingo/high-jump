#include "sqlite.h"

#include <iostream>

int sqlite::exec(const char *sql)
{
    int  ms   = 100;
    auto conn = _pool.acquire(ms);
    if(!conn || !conn->is_open())
        return DB_ERR_SQLITE_GET_CONN_FAIL;

    if(!conn->exec(sql))
        return DB_ERR_SQLITE_EXEC_FAIL;

    return OK;
}

int sqlite::query(std::vector<std::vector<std::string> > &outs, const char *sql)
{
    int  ms   = 100;
    auto conn = _pool.acquire(ms);
    if(!conn || !conn->is_open())
        return DB_ERR_SQLITE_GET_CONN_FAIL;

    outs = conn->query(sql, &sqlite::_query_cb);
    return OK;
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

int sqlite::_query_cb(void *in, int argc, char **argv, char **col_name)
{
    auto *rows = static_cast<std::vector<std::vector<std::string> > *>(in);
    std::vector<std::string> row;
    for(int i = 0; i < argc; ++i)
        row.push_back(argv[i] ? argv[i] : "");

    rows->push_back(row);
    return 0;
}