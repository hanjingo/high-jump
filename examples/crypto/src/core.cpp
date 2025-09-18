#include "core.h"
#include "comm.h"

#include <libcpp/util/defer.hpp>

// crypto core
crypto_core::crypto_core()
{
    load();
}

crypto_core::~crypto_core()
{
    unload();
}

err_t crypto_core::load()
{
#if defined(_WIN32)
    _dll = dll_open("crypto_core.dll", DLL_RTLD_NOW);
#else
    _dll = dll_open("libcrypto_core.so", DLL_RTLD_NOW);
#endif
    if (_dll == nullptr)
        return error(CRYPTO_ERR_CORE_LOAD_FAIL);

    _require = static_cast<crypto_api>(dll_get(_dll, "crypto_require"));
    _release = static_cast<crypto_api>(dll_get(_dll, "crypto_release"));
    _version = static_cast<crypto_api>(dll_get(_dll, "crypto_version"));
    _init = static_cast<crypto_api>(dll_get(_dll, "crypto_init"));
    _quit = static_cast<crypto_api>(dll_get(_dll, "crypto_quit"));
    _encrypt = static_cast<crypto_api>(dll_get(_dll, "crypto_encrypt"));
    _decrypt = static_cast<crypto_api>(dll_get(_dll, "crypto_decrypt"));
    _keygen = static_cast<crypto_api>(dll_get(_dll, "crypto_keygen"));
    return err_t();
}

err_t crypto_core::unload()
{
    auto err = quit();
    if (err)
        return err;

    _require = nullptr;
    _release = nullptr;
    _version = nullptr;
    _init = nullptr;
    _quit = nullptr;
    _encrypt = nullptr;
    _decrypt = nullptr;
    _keygen = nullptr;
    if (_dll)
    {
        dll_close(_dll);
        _dll = nullptr;
    }

    return err_t();
}

err_t crypto_core::reload()
{
    auto err = unload();
    if (err)
        return err;

    return load();
}

err_t crypto_core::version(int& major, int& minor, int& patch)
{
    if (_version == nullptr)
        return error(CRYPTO_ERR_CORE_NOT_LOAD);

    auto param = new crypto_param_version{};
    DEFER(delete param; param = nullptr;);

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _version(ctx);
    major = param->major;
    minor = param->minor;
    patch = param->patch;
    return err_t();
}

err_t crypto_core::init(
    const size_t data_pool_size)
{
    if (_init == nullptr)
        return error(CRYPTO_ERR_CORE_NOT_LOAD);

    auto param = new crypto_param_init{};
    DEFER(delete param; param = nullptr;);

    param->data_pool_size = data_pool_size;

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _init(ctx);
    if (param->result != CRYPTO_OK)
        return error(param->result);

    return err_t();
}

err_t crypto_core::quit()
{
    if (_quit == nullptr)
        return error(CRYPTO_ERR_CORE_NOT_LOAD);

    auto param = new crypto_param_quit{};
    DEFER(delete param; param = nullptr;);

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _quit(ctx);
    if (param->result != CRYPTO_OK)
        return error(param->result);

    return err_t();
}

err_t crypto_core::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    auto value = require(CRYPTO_PARAM_ENCRYPT);
    if (value == nullptr)
        return error(CRYPTO_ERR_REQUIRE_FAIL);

    auto param = static_cast<crypto_param_encrypt*>(value);
    DEFER(release(CRYPTO_PARAM_ENCRYPT, param); param = nullptr;);
    if (!out.empty())
    {
        strncpy_s(param->out, out.size(), out.c_str(), CRYPTO_OUTPUT_BUF);
        *param->out_len = out.size();
    }
    param->in = in.c_str();
    param->content = content.c_str();
    param->algo = algo.c_str();
    param->mode = mode.c_str();
    param->key = key.c_str();
    param->padding = padding.c_str();
    param->iv = iv.c_str();
    param->fmt = fmt.c_str();

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _encrypt(ctx);
    if (param->result != CRYPTO_OK)
        return error(param->result);

    out.assign(param->out, *param->out_len);
    return err_t();
}

err_t crypto_core::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    auto value = require(CRYPTO_PARAM_DECRYPT);
    if (value == nullptr)
        return error(CRYPTO_ERR_REQUIRE_FAIL);

    auto param = static_cast<crypto_param_decrypt*>(value);
    DEFER(release(CRYPTO_PARAM_DECRYPT, param); param = nullptr;);
    if (!out.empty())
    {
        strncpy_s(param->out, out.size(), out.c_str(), CRYPTO_OUTPUT_BUF);
        *param->out_len = out.size();
    }
    param->in = in.c_str();
    param->content = content.c_str();
    param->algo = algo.c_str();
    param->mode = mode.c_str();
    param->key = key.c_str();
    param->passwd = passwd.c_str();
    param->padding = padding.c_str();
    param->iv = iv.c_str();
    param->fmt = fmt.c_str();

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _decrypt(ctx);
    if (param->result != CRYPTO_OK)
        return error(param->result);

    out.assign(param->out, *param->out_len);
    return err_t();
}

err_t crypto_core::keygen(
    std::vector<std::string>& outs,
    const std::string& algo,
    const std::string& fmt,
    const std::string& mode,
    const size_t bits)
{
    auto value = require(CRYPTO_PARAM_KEYGEN);
    if (value == nullptr)
        return error(CRYPTO_ERR_REQUIRE_FAIL);

    auto param = static_cast<crypto_param_keygen*>(value);
    DEFER(release(CRYPTO_PARAM_KEYGEN, param); param = nullptr;);
    param->algo = algo.c_str();
    param->fmt = fmt.c_str();
    param->mode = mode.c_str();
    param->bits = bits;

    crypto_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(crypto_context);
    _keygen(ctx);
    if (param->result != CRYPTO_OK)
        return error(param->result);

    for (size_t i = 0; i < CRYPTO_MAX_KEY_NUM; i++)
    {
        if (param == nullptr || param->out == nullptr || param->out_len == nullptr)
            continue;
        if (param->out_len[i] == nullptr || *param->out_len[i] == 0)
            continue;

        outs.emplace_back(std::string(param->out[i], *param->out_len[i]));
    }

    return err_t();
}

// db core
db_core::db_core()
{
    load();
}

db_core::~db_core()
{
    unload();
}

err_t db_core::load()
{
#if defined(_WIN32)
    _dll = dll_open("db_core.dll", DLL_RTLD_NOW);
#else
    _dll = dll_open("libdb_core.so", DLL_RTLD_NOW);
#endif
    if (_dll == nullptr)
        return error(DB_ERR_CORE_LOAD_FAIL);

    _version = static_cast<db_api>(dll_get(_dll, "db_version"));
    _init = static_cast<db_api>(dll_get(_dll, "db_init"));
    _quit = static_cast<db_api>(dll_get(_dll, "db_quit"));
    return err_t();
}

err_t db_core::unload()
{
    auto err = quit();
    if (err)
        return err;

    _version = nullptr;
    _init = nullptr;
    _quit = nullptr;
    if (_dll)
    {
        dll_close(_dll);
        _dll = nullptr;
    }

    return err_t();
}

err_t db_core::reload()
{
    auto err = unload();
    if (err)
        return err;

    return load();
}

err_t db_core::version(int& major, int& minor, int& patch)
{
    if (_version == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    auto param = new db_param_version{};
    DEFER(delete param; param = nullptr;);

    db_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(db_context);
    _version(ctx);
    major = param->major;
    minor = param->minor;
    patch = param->patch;
    return err_t();
}

err_t db_core::init()
{
    if (_init == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    auto param = new db_param_init{};
    DEFER(delete param; param = nullptr;);

    param->sqlite_conn_capa = 10;
    param->sqlite_db_path = "dict.db";

    db_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(db_context);
    _init(ctx);
    if (param->result != DB_OK)
        return error(param->result);

    return err_t();
}

err_t db_core::quit()
{
    if (_quit == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    return err_t();
}

err_t db_core::add(
    const std::string& key)
{
    if (_exec == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    return err_t();
}

err_t db_core::select(
    const std::string& key,
    const int num)
{
    if (_exec == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    return err_t();
}