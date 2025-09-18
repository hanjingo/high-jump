#ifndef SQLITE_H
#define SQLITE_H

#include <string>
#include <memory>

#include <libcpp/db/sqlite.hpp>
#include <libcpp/db/db_conn_pool.hpp>

#include "comm.h"
#include "api.h"

class sqlite : public db
{
public:
    using conn_t = libcpp::sqlite;
    using conn_ptr_t = std::shared_ptr<conn_t>;

public:
    sqlite(const char* path, const std::size_t capa)
        : _path{path}
        , _pool{
            capa,
            [this]()->conn_ptr_t { return _make_conn(); },
            [this](conn_ptr_t conn)->bool { return _check_conn(conn); }}
    {
    }
    ~sqlite()
    {
    }

    const std::string type() override { return "sqlite"; }

    int exec(const char* sql) override;

private:
    conn_ptr_t _make_conn();
    bool _check_conn(conn_ptr_t conn);

private:
    sqlite(const sqlite&) = delete;
    sqlite& operator=(const sqlite&) = delete;
    sqlite(sqlite&&) = delete;
    sqlite& operator=(sqlite&&) = delete;

private:
    std::string                  _path;
    libcpp::db_conn_pool<conn_t> _pool;
};

#endif