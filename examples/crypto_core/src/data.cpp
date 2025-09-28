#include "data.h"

#include <hj/util/once.hpp>

void data_mgr::init(size_t pool_size)
{
    ONCE(
        pool_size = (pool_size == 0) ? 1 : pool_size;
        pool_size = (pool_size > CRYPTO_MAX_DATA_POOL_SIZE) ? CRYPTO_MAX_DATA_POOL_SIZE : pool_size;
        for (size_t i = 0; i < pool_size; i++)
        {
            // param_keygen
            {
                auto* param = _keygen_pool.allocate();
                _init(param);
                _keygen_pool.push(param);
}

// param_encrypt
{
    auto *param = _encrypt_pool.allocate();
    _init(param);
    _encrypt_pool.push(param);
}

// param_decrypt
{
    auto *param = _decrypt_pool.allocate();
    _init(param);
    _decrypt_pool.push(param);
}
}
    );
    }

    const int data_mgr::size(const int typ)
    {
        switch(typ)
        {
            case CRYPTO_PARAM_KEYGEN: {
                return _keygen_pool.size();
            }
            case CRYPTO_PARAM_ENCRYPT: {
                return _encrypt_pool.size();
            }
            case CRYPTO_PARAM_DECRYPT: {
                return _decrypt_pool.size();
            }
            default:
                return 0;
        }
    }

    void *data_mgr::require(const int typ)
    {
        switch(typ)
        {
            case CRYPTO_PARAM_KEYGEN: {
                return _keygen_pool.pop();
            }
            case CRYPTO_PARAM_ENCRYPT: {
                return _encrypt_pool.pop();
            }
            case CRYPTO_PARAM_DECRYPT: {
                return _decrypt_pool.pop();
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
            case CRYPTO_PARAM_KEYGEN: {
                auto param = static_cast<crypto_param_keygen *>(value);
                _reset(param);
                _keygen_pool.push(param);
                break;
            }
            case CRYPTO_PARAM_ENCRYPT: {
                auto param = static_cast<crypto_param_encrypt *>(value);
                _reset(param);
                _encrypt_pool.push(param);
                break;
            }
            case CRYPTO_PARAM_DECRYPT: {
                auto param = static_cast<crypto_param_decrypt *>(value);
                _reset(param);
                _decrypt_pool.push(param);
                break;
            }
            default:
                break;
        }
    }

    void data_mgr::_init(crypto_param_keygen *param)
    {
        if(param == nullptr)
            return;

        param->out     = new char *[CRYPTO_MAX_KEY_NUM];
        param->out_len = new size_t *[CRYPTO_MAX_KEY_NUM];
        for(size_t j = 0; j < CRYPTO_MAX_KEY_NUM; j++)
        {
            param->out[j]     = new char[CRYPTO_MAX_KEY_SIZE];
            param->out_len[j] = new size_t(0);
            memset(param->out[j], 0, CRYPTO_MAX_KEY_SIZE);
        }
        param->algo   = nullptr;
        param->fmt    = nullptr;
        param->mode   = nullptr;
        param->bits   = 0;
        param->result = CRYPTO_ERR_FAIL;
    }

    void data_mgr::_reset(crypto_param_keygen *param)
    {
        if(param == nullptr)
            return;

        for(size_t i = 0; i < CRYPTO_MAX_KEY_NUM; i++)
        {
            if(param->out[i] != nullptr)
                memset(param->out[i], 0, CRYPTO_MAX_KEY_SIZE);

            if(param->out_len[i] != nullptr)
                *(param->out_len[i]) = 0;
        }

        param->algo   = nullptr;
        param->fmt    = nullptr;
        param->mode   = nullptr;
        param->bits   = 0;
        param->result = CRYPTO_ERR_FAIL;
    }

    void data_mgr::_init(crypto_param_encrypt *param)
    {
        if(param == nullptr)
            return;

        param->result  = CRYPTO_ERR_FAIL;
        param->out     = new char[CRYPTO_MAX_OUTPUT_SIZE];
        param->out_len = new size_t(0);
        param->in      = nullptr;
        param->content = nullptr;
        param->algo    = nullptr;
        param->mode    = nullptr;
        param->key     = nullptr;
        param->padding = nullptr;
        param->iv      = nullptr;
        param->fmt     = nullptr;
    }

    void data_mgr::_reset(crypto_param_encrypt *param)
    {
        if(param == nullptr)
            return;

        param->result = CRYPTO_ERR_FAIL;
        if(param->out != nullptr)
            memset((char *) param->out, 0, CRYPTO_MAX_OUTPUT_SIZE);
        if(param->out_len != nullptr)
            *(param->out_len) = 0;
        param->in      = nullptr;
        param->content = nullptr;
        param->algo    = nullptr;
        param->mode    = nullptr;
        param->key     = nullptr;
        param->padding = nullptr;
        param->iv      = nullptr;
        param->fmt     = nullptr;
    }

    void data_mgr::_init(crypto_param_decrypt *param)
    {
        if(param == nullptr)
            return;

        param->result  = CRYPTO_ERR_FAIL;
        param->out     = new char[CRYPTO_MAX_OUTPUT_SIZE];
        param->out_len = new size_t(0);
        param->in      = nullptr;
        param->content = nullptr;
        param->algo    = nullptr;
        param->mode    = nullptr;
        param->key     = nullptr;
        param->padding = nullptr;
        param->iv      = nullptr;
        param->fmt     = nullptr;
    }

    void data_mgr::_reset(crypto_param_decrypt *param)
    {
        if(param == nullptr)
            return;

        param->result = CRYPTO_ERR_FAIL;
        if(param->out != nullptr)
            memset((char *) param->out, 0, CRYPTO_MAX_OUTPUT_SIZE);
        if(param->out_len != nullptr)
            *(param->out_len) = 0;
        param->in      = nullptr;
        param->content = nullptr;
        param->algo    = nullptr;
        param->mode    = nullptr;
        param->key     = nullptr;
        param->padding = nullptr;
        param->iv      = nullptr;
        param->fmt     = nullptr;
    }