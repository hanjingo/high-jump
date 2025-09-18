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

    std::vector<std::string> supported_dbs();

    int add(std::unique_ptr<db>&& db);

private:
    std::vector<std::unique_ptr<db>> _dbs;
};

#endif