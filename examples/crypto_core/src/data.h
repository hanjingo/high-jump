#ifndef DATA_H
#define DATA_H

#include <hj/sync/object_pool.hpp>

#include "api.h"

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
        static std::vector<int> types{
            CRYPTO_PARAM_KEYGEN, CRYPTO_PARAM_DECRYPT, CRYPTO_PARAM_ENCRYPT};
        return types;
    }

    void init(size_t pool_size);
    const int size(const int typ);
    void* require(const int typ);
    void release(const int typ, void* value);

private:
    void _init(crypto_param_keygen* param);
    void _reset(crypto_param_keygen* param);

    void _init(crypto_param_encrypt* param);
    void _reset(crypto_param_encrypt* param);

    void _init(crypto_param_decrypt* param);
    void _reset(crypto_param_decrypt* param);

private:
    data_mgr() = default;
    ~data_mgr() = default;
    data_mgr(const data_mgr&) = delete;
    data_mgr& operator=(const data_mgr&) = delete;
    data_mgr(data_mgr&&) = delete;
    data_mgr& operator=(data_mgr&&) = delete;

private:
    hj::object_pool<crypto_param_keygen> _keygen_pool;
    hj::object_pool<crypto_param_encrypt> _encrypt_pool;
    hj::object_pool<crypto_param_decrypt> _decrypt_pool;
};

#endif