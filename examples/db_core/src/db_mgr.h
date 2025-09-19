#ifndef DB_MGR_H
#define DB_MGR_H

#include <memory>
#include <vector>
#include <string>

#include "api.h"
#include "comm.h"
#include "sqlite.h"

class db_mgr
{
public:
    db_mgr()
        : _dbs{}
    {
    }
    ~db_mgr()
    {
        _dbs.clear();
    }

    static db_mgr& instance()
    {
        static db_mgr inst;
        return inst;
    }

    const std::vector<std::string>& supported_db_types();

    int add(std::unique_ptr<db>&& db);
    int exec(
        const std::string& db_id, 
        const std::string& sql);
    int query(
        std::vector<std::vector<std::string>>& outs, // for example: [["name", "age"], [...], ...]
        const std::string& db_id, 
        const std::string& sql);

private:
    std::vector<std::unique_ptr<db>> _dbs;
};

#endif