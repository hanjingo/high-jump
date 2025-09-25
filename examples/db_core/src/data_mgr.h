#ifndef DATA_MGR_H
#define DATA_MGR_H

#include <hj/sync/object_pool.hpp>

#include "db_core.h"

class data_mgr
{
public:
    static data_mgr& instance()
    {
        static data_mgr inst;
        return inst;
    }

    inline std::vector<int>& supported_types()
    {
        static std::vector<int> types{DB_PARAM_EXEC, };
        return types;
    }

    void init(size_t pool_size);
    const int size(const int typ);
    void* require(const int typ);
    void release(const int typ, void* value);

private:
    void _init(db_param_exec* param);
    void _reset(db_param_exec* param);

    void _init(db_param_query* param);
    void _reset(db_param_query* param);

private:
    data_mgr() = default;
    ~data_mgr() = default;
    data_mgr(const data_mgr&) = delete;
    data_mgr& operator=(const data_mgr&) = delete;
    data_mgr(data_mgr&&) = delete;
    data_mgr& operator=(data_mgr&&) = delete;

private:
    hj::object_pool<db_param_exec> _exec_pool;
    hj::object_pool<db_param_query> _query_pool;
};

#endif