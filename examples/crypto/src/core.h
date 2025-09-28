#ifndef CORE_H
#define CORE_H

#include <hj/os/dll.h>
#include <hj/log/log.hpp>
#include <hj/util/defer.hpp>

#include "comm.h"
#include "examples/crypto_core/src/crypto_core.h"
#include "examples/db_core/src/db_core.h"

// // virtual base class
// class core
// {
// public:
//     virtual std::string name() const = 0;
//     virtual std::string version() const = 0;
//     virtual void init() = 0;
//     virtual void quit() = 0;
// };

// crypto core
class crypto_core
{
  public:
    crypto_core();
    ~crypto_core();

    err_t load();
    err_t unload();
    err_t reload();

    // API
    err_t version(int &major, int &minor, int &patch);
    err_t init(const size_t data_pool_size = 1);
    err_t quit();
    err_t encrypt(std::string       &out,
                  const std::string &in,
                  const std::string &content,
                  const std::string &algo,
                  const std::string &mode,
                  const std::string &key,
                  const std::string &padding,
                  const std::string &iv,
                  const std::string &fmt);
    err_t decrypt(std::string       &out,
                  const std::string &in,
                  const std::string &content,
                  const std::string &algo,
                  const std::string &mode,
                  const std::string &key,
                  const std::string &passwd,
                  const std::string &padding,
                  const std::string &iv,
                  const std::string &fmt);
    err_t keygen(std::vector<std::string> &outs,
                 const std::string        &algo,
                 const std::string        &fmt,
                 const std::string        &mode,
                 const size_t              bits);

  protected:
    void *require(int typ)
    {
        if(_require == nullptr)
            return nullptr;

        sdk_context           ctx;
        crypto_param_require *param = new crypto_param_require{};
        DEFER(delete param; param = nullptr;);
        param->type  = typ;
        param->value = nullptr;

        ctx.user_data = param;
        ctx.cb        = nullptr;
        ctx.sz        = sizeof(sdk_context);
        _require(ctx);
        return param->value;
    }

    void release(int typ, void *value)
    {
        if(_release == nullptr)
            return;

        sdk_context           ctx;
        crypto_param_release *param = new crypto_param_release{};
        DEFER(delete param; param = nullptr;);
        param->type  = typ;
        param->value = value;

        ctx.user_data = param;
        ctx.cb        = nullptr;
        ctx.sz        = sizeof(sdk_context);
        _release(ctx);
    }

  private:
    void   *_dll     = nullptr;
    sdk_api _require = nullptr;
    sdk_api _release = nullptr;
    sdk_api _version = nullptr;
    sdk_api _init    = nullptr;
    sdk_api _quit    = nullptr;
    sdk_api _encrypt = nullptr;
    sdk_api _decrypt = nullptr;
    sdk_api _keygen  = nullptr;
};

// db core
class db_core
{
  public:
    db_core();
    ~db_core();

    err_t load();
    err_t unload();
    err_t reload();

    // API
    err_t version(int &major, int &minor, int &patch);
    err_t init();
    err_t quit();

    err_t exec(const std::string &db_id, const std::string &sql);
    err_t add(const std::string &db_id, const std::string &key);
    err_t query(std::vector<std::vector<std::string> > &outs,
                const std::string                      &db_id,
                const std::string                      &sql);
    err_t query(std::vector<std::vector<std::string> > &outs,
                const std::string                      &db_id,
                const std::string                      &tbl,
                const std::vector<std::string>         &contents,
                const int                               count);

  protected:
    void *require(int typ)
    {
        if(_require == nullptr)
            return nullptr;

        sdk_context       ctx;
        db_param_require *param = new db_param_require{};
        DEFER(delete param; param = nullptr;);
        param->type  = typ;
        param->value = nullptr;

        ctx.user_data = param;
        ctx.cb        = nullptr;
        ctx.sz        = sizeof(sdk_context);
        _require(ctx);
        return param->value;
    }

    void release(int typ, void *value)
    {
        if(_release == nullptr)
            return;

        sdk_context       ctx;
        db_param_release *param = new db_param_release{};
        DEFER(delete param; param = nullptr;);
        param->type  = typ;
        param->value = value;

        ctx.user_data = param;
        ctx.cb        = nullptr;
        ctx.sz        = sizeof(sdk_context);
        _release(ctx);
    }

  private:
    void   *_dll     = nullptr;
    sdk_api _require = nullptr;
    sdk_api _release = nullptr;
    sdk_api _version = nullptr;
    sdk_api _init    = nullptr;
    sdk_api _quit    = nullptr;
    sdk_api _exec    = nullptr;
    sdk_api _query   = nullptr;
};

#endif