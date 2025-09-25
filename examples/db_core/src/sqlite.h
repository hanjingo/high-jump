#ifndef SQLITE_H
#define SQLITE_H

#include <string>
#include <memory>

#include <hj/db/sqlite.hpp>
#include <hj/db/db_conn_pool.hpp>

#include "comm.h"
#include "db_core.h"

class sqlite : public db
{
public:
    using conn_t = hj::sqlite;
    using conn_ptr_t = std::shared_ptr<conn_t>;

public:
    sqlite(const char* id, const char* path, const std::size_t capa)
        : _id{id}
        , _path{path}
        , _pool{
            capa,
            [this]()->conn_ptr_t { return _make_conn(); },
            [this](conn_ptr_t conn)->bool { return _check_conn(conn); }}
    {
    }
    ~sqlite()
    {
    }

    const std::string id() override { return _id; }

    int exec(const char* sql) override;
    int query(
        std::vector<std::vector<std::string>>& outs, 
        const char* sql) override;

private:
    sqlite(const sqlite&) = delete;
    sqlite& operator=(const sqlite&) = delete;
    sqlite(sqlite&&) = delete;
    sqlite& operator=(sqlite&&) = delete;

    conn_ptr_t _make_conn();
    bool _check_conn(conn_ptr_t conn);

    static int _query_cb(void* in, int argc, char** argv, char** col_name);

private:
    std::string                  _id;
    std::string                  _path;
    hj::db_conn_pool<conn_t> _pool;
};

#endif