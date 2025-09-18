#include "db_mgr.h"

std::vector<std::string> db_mgr::supported_dbs()
{
    std::vector<std::string> ret;
    for (const auto& db : _dbs)
        ret.push_back(db->type());

    return ret;
}

int db_mgr::add(std::unique_ptr<db>&& elem)
{
    for (const auto& e : _dbs)
        if (e->type() == elem->type())
            return DB_ERR_FAIL;

    _dbs.emplace_back(std::move(elem));
    return DB_OK;
}