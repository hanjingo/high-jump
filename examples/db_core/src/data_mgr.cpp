#include "data_mgr.h"

#include <iostream>
#include <hj/util/once.hpp>

void data_mgr::init(size_t pool_size)
{
    ONCE(
        pool_size = (pool_size == 0) ? 1 : pool_size;
        pool_size = (pool_size > DB_MAX_DATA_POOL_SIZE) ? DB_MAX_DATA_POOL_SIZE : pool_size;
        for (size_t i = 0; i < pool_size; i++)
        {
            // param_exec
            {
                auto* param = _exec_pool.allocate();
                _init(param);
                _exec_pool.push(param);
}

// param_query
{
    auto *param = _query_pool.allocate();
    _init(param);
    _query_pool.push(param);
}
}
    );
    }

    const int data_mgr::size(const int typ)
    {
        switch(typ)
        {
            case DB_PARAM_EXEC: {
                return _exec_pool.size();
            }
            case DB_PARAM_QUERY: {
                return _query_pool.size();
            }
            default:
                return 0;
        }
    }

    void *data_mgr::require(const int typ)
    {
        switch(typ)
        {
            case DB_PARAM_EXEC: {
                return _exec_pool.pop();
            }
            case DB_PARAM_QUERY: {
                return _query_pool.pop();
            }
            default: {
                return nullptr;
            }
        }
    }

    void data_mgr::release(const int typ, void *value)
    {
        if(value == nullptr)
            return;

        switch(typ)
        {
            case DB_PARAM_EXEC: {
                auto param = static_cast<db_param_exec *>(value);
                _reset(param);
                _exec_pool.push(param);
                break;
            }
            case DB_PARAM_QUERY: {
                auto param = static_cast<db_param_query *>(value);
                _reset(param);
                _query_pool.push(param);
                break;
            }
            default:
                break;
        }
    }

    void data_mgr::_init(db_param_exec *param)
    {
        if(param == nullptr)
            return;

        param->db_id  = nullptr;
        param->sql    = nullptr;
        param->result = DB_ERR_FAIL;
    }

    void data_mgr::_reset(db_param_exec *param)
    {
        if(param == nullptr)
            return;

        param->db_id  = nullptr;
        param->sql    = nullptr;
        param->result = DB_ERR_FAIL;
    }

    void data_mgr::_init(db_param_query *param)
    {
        if(param == nullptr)
            return;

        // [["xx", "xxx", ...], ...]
        // [[1, 2, ...], ...]
        param->out     = new char **[DB_MAX_QUERY_OUTPUT_NUM_LVL1];
        param->out_len = new size_t **[DB_MAX_QUERY_OUTPUT_NUM_LVL1];
        for(size_t i = 0; i < DB_MAX_QUERY_OUTPUT_NUM_LVL1; i++)
        {
            param->out[i]     = new char *[DB_MAX_QUERY_OUTPUT_NUM_LVL2];
            param->out_len[i] = new size_t *[DB_MAX_QUERY_OUTPUT_NUM_LVL2];
            for(size_t j = 0; j < DB_MAX_QUERY_OUTPUT_NUM_LVL2; j++)
            {
                param->out[i][j]     = new char[DB_MAX_QUERY_OUTPUT_SIZE];
                param->out_len[i][j] = new size_t(0);
                memset(param->out[i][j], 0, DB_MAX_QUERY_OUTPUT_SIZE);
            }
        }

        param->db_id  = nullptr;
        param->sql    = nullptr;
        param->result = DB_ERR_FAIL;
    }

    void data_mgr::_reset(db_param_query *param)
    {
        if(param == nullptr)
            return;

        for(size_t i = 0; i < DB_MAX_QUERY_OUTPUT_NUM_LVL1; i++)
        {
            for(size_t j = 0; j < DB_MAX_QUERY_OUTPUT_NUM_LVL2; j++)
            {
                if(param->out != nullptr && param->out[i] != nullptr
                   && param->out[i][j] != nullptr)
                {
                    memset(param->out[i][j], 0, DB_MAX_QUERY_OUTPUT_SIZE);
                }

                if(param->out_len != nullptr && param->out_len[i] != nullptr
                   && param->out_len[i][j] != nullptr)
                {
                    *(param->out_len[i][j]) = 0;
                }
            }
        }

        param->db_id  = nullptr;
        param->sql    = nullptr;
        param->result = DB_ERR_FAIL;
    }