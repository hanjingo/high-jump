#include "db_mgr.h"

const std::vector<std::string>& db_mgr::supported_db_types()
{
    static std::vector<std::string> ret{"sqlite"};
    return ret;
}

int db_mgr::add(std::unique_ptr<db>&& elem)
{
    for (const auto& e : _dbs)
        if (e->id() == elem->id())
            return DB_ERR_DB_EXISTED;

    _dbs.emplace_back(std::move(elem));
    return DB_OK;
}

int db_mgr::exec(
    const std::string& db_id, 
    const std::string& sql)
{
    for (const auto& e : _dbs)
    {
        if (e->id() != db_id)
            continue;
        
        return e->exec(sql.c_str());
    }

    return DB_ERR_DB_NOT_EXIST;
}

int db_mgr::query(
    std::vector<std::vector<std::string>>& outs,
    const std::string& db_id, 
    const std::string& sql)
{
    for (const auto& e : _dbs)
    {
        if (e->id() != db_id)
            continue;

        return e->query(outs, sql.c_str());
    }

    return DB_ERR_DB_NOT_EXIST;
}