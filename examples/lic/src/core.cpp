#include "core.h"

#include <hj/util/defer.hpp>
#include <hj/util/string_util.hpp>

// ------------------------------ lic core ------------------------------
lic_core::lic_core()
{
    auto err = load();
    if(err)
        throw std::runtime_error("lic_core load fail with err: "
                                 + err.message());
}

lic_core::~lic_core()
{
    unload();
}

err_t lic_core::load()
{
#if defined(_WIN32)
    _dll = dll_open("lic_core.dll", DLL_RTLD_NOW);
#else
    _dll = dll_open("liblic_core.so", DLL_RTLD_NOW);
#endif
    if(_dll == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    _version    = static_cast<sdk_api>(dll_get(_dll, "lic_version"));
    _init       = static_cast<sdk_api>(dll_get(_dll, "lic_init"));
    _quit       = static_cast<sdk_api>(dll_get(_dll, "lic_quit"));
    _add_issuer = static_cast<sdk_api>(dll_get(_dll, "lic_add_issuer"));
    _issue      = static_cast<sdk_api>(dll_get(_dll, "lic_issue"));
    _keygen     = static_cast<sdk_api>(dll_get(_dll, "lic_keygen"));
    return err_t();
}

err_t lic_core::unload()
{
    auto err = quit();
    if(err)
        return err;


    _version    = nullptr;
    _init       = nullptr;
    _quit       = nullptr;
    _add_issuer = nullptr;
    _issue      = nullptr;
    _keygen     = nullptr;
    if(_dll)
    {
        dll_close(_dll);
        _dll = nullptr;
    }

    return err_t();
}

err_t lic_core::reload()
{
    auto err = unload();
    if(err)
        return err;

    return load();
}

err_t lic_core::version(int &major, int &minor, int &patch)
{
    if(_version == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_version{};
    DEFER(delete param; param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb        = nullptr;
    ctx.sz        = sizeof(sdk_context);
    _version(ctx);
    major = param->major;
    minor = param->minor;
    patch = param->patch;
    return err_t();
}

err_t lic_core::init()
{
    if(_init == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_init{};
    DEFER(delete param; param = nullptr;);

    param->result = ERR_FAIL;

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb        = nullptr;
    ctx.sz        = sizeof(sdk_context);
    _init(ctx);
    if(param->result != OK)
        return error(param->result, "lic");

    // add default none issuer
    auto err = add_issuer(LIC_ISSUER, "none", 1);
    return err;
}

err_t lic_core::quit()
{
    if(_quit == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_quit{};
    DEFER(delete param; param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb        = nullptr;
    ctx.sz        = sizeof(sdk_context);
    _quit(ctx);
    if(param->result != OK)
        return error(param->result, "lic");

    return err_t();
}

err_t lic_core::add_issuer(const std::string &issuer,
                           const std::string &algo,
                           const int          times,
                           const std::string &key_str)
{
    if(_quit == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    // parse keys
    std::vector<std::string> keys;
    if(!key_str.empty())
        keys = hj::string::split(key_str, ",");

    auto param       = new lic_param_add_issuer{};
    param->issuer_id = issuer.c_str();
    param->algo      = algo.c_str();
    param->times     = times;
    param->keys      = new const char *[LIC_MAX_KEY_NUM] { nullptr };
    param->keys_num  = 0;
    for(auto e : keys)
    {
        if(param->keys_num >= LIC_MAX_KEY_NUM)
            break;

        param->keys[param->keys_num] = e.c_str();
        param->keys_num++;
    }
    DEFER(for(size_t i = 0; i < param->keys_num; i++) {
        if(!param->keys[i])
            continue;

        param->keys[i] = nullptr;
    } delete[] param->keys;
          param->keys = nullptr;
          delete param;
          param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb        = nullptr;
    ctx.sz        = sizeof(sdk_context);
    _add_issuer(ctx);
    if(param->result != OK)
        return error(param->result, "lic");

    return err_t();
}

err_t lic_core::issue(std::string       &out,
                      const std::string &algo,
                      const std::string &licensee,
                      const std::string &issuer,
                      const int          time, // days
                      const std::string &claim_str)
{
    if(_issue == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto claims = parse_claims(claim_str);
    if(claims.size() * 2 > LIC_MAX_CLAIM_NUM)
        return error(ERR_LIC_CORE_CLAIM_NUM_EXCEED);

    auto param        = new lic_param_issue{};
    param->out        = new char[LIC_MAX_OUTPUT_SIZE]{0};
    param->out_len    = new size_t{0};
    param->algo       = algo.c_str();
    param->licensee   = licensee.c_str();
    param->issuer_id  = issuer.c_str();
    param->time       = time;
    param->claims     = nullptr;
    param->claims_num = 0;

    // set output param
    if(!out.empty())
    {
        strncpy_s(param->out, LIC_MAX_OUTPUT_SIZE, out.c_str(), out.size());
        *param->out_len = out.size();
    }

    // parse claims
    param->claims = new const char *[claims.size() * 2] { nullptr };
    for(auto e : claims)
    {
        param->claims[param->claims_num] = e.first.c_str();
        param->claims_num++;
        param->claims[param->claims_num] = e.second.c_str();
        param->claims_num++;
    }

    DEFER(delete[] param->out; param->out = nullptr; delete param->out_len;
          param->out_len                  = nullptr;
          for(size_t i = 0; i < param->claims_num; i++) {
              if(!param->claims[i])
                  continue;

              delete[] param->claims[i];
              param->claims[i] = nullptr;
          } delete param;
          param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb        = nullptr;
    ctx.sz        = sizeof(sdk_context);
    _issue(ctx);
    if(param->result != OK)
        return error(param->result, "lic");

    out.assign(param->out, *param->out_len);
    return err_t();
}