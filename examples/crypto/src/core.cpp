#include "core.h"
#include "comm.h"

#include <libcpp/util/defer.hpp>

// ------------------------------ crypto core ------------------------------
crypto_core::crypto_core()
{
    auto err = load();
    if (err)
        throw std::runtime_error("crypto_core load fail with err: " + err.message());
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
        strncpy_s(param->out, CRYPTO_MAX_OUTPUT_SIZE, out.c_str(), out.size());
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
        strncpy_s(param->out, CRYPTO_MAX_OUTPUT_SIZE, out.c_str(), out.size());
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

// ------------------------------ db core ------------------------------
db_core::db_core()
{
    auto err = load();
    if (err)
        throw std::runtime_error("db_core load fail with err: " + err.message());
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

    _require = static_cast<db_api>(dll_get(_dll, "db_require"));
    _release = static_cast<db_api>(dll_get(_dll, "db_release"));
    _version = static_cast<db_api>(dll_get(_dll, "db_version"));
    _init = static_cast<db_api>(dll_get(_dll, "db_init"));
    _quit = static_cast<db_api>(dll_get(_dll, "db_quit"));
    _exec = static_cast<db_api>(dll_get(_dll, "db_exec"));
    _query = static_cast<db_api>(dll_get(_dll, "db_query"));
    return err_t();
}

err_t db_core::unload()
{
    auto err = quit();
    if (err)
        return err;

    _require = nullptr;
    _release = nullptr;
    _version = nullptr;
    _init = nullptr;
    _quit = nullptr;
    _exec = nullptr;
    _query = nullptr;
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

    param->result = DB_ERR_FAIL;

    std::string opt_str = R"(
[data_pool1]
type=data_pool
data_pool_size=1

[dict1]
type=sqlite
db_id=dict
db_path=dict.db
db_conn_capa=1)";// ini style
    param->option = opt_str.c_str(); 

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

    auto param = new db_param_quit{};
    DEFER(delete param; param = nullptr;);

    db_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(db_context);
    _quit(ctx);
    if (param->result != DB_OK)
        return error(param->result);

    return err_t();
}

err_t db_core::exec(
    const std::string& db_id,
    const std::string& sql)
{
    if (_exec == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    auto value = require(DB_PARAM_EXEC);
    if (value == nullptr)
        return error(DB_ERR_REQUIRE_FAIL);

    auto param = static_cast<db_param_exec*>(value);
    DEFER(release(DB_PARAM_EXEC, param); param = nullptr;);
    param->db_id = db_id.c_str();
    param->sql = sql.c_str();

    db_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(db_context);
    _exec(ctx);
    if (param->result != DB_OK)
        return error(param->result);

    return err_t();
}

err_t  db_core::add(
    const std::string& db_id,
    const std::string& key)
{
    std::vector<std::string> outs;
    if (db_id == "dict")
    {
        std::string sql = "INSERT INTO passwords (password) VALUES ('" + key + "')";
        auto err = exec(db_id, sql);
        if (err)
            return err;
    }

    return err_t();
}

err_t db_core::query(
    std::vector<std::vector<std::string>>& outs,
    const std::string& db_id,
    const std::string& sql)
{
    if (_query == nullptr)
        return error(DB_ERR_CORE_NOT_LOAD);

    auto value = require(DB_PARAM_QUERY);
    if (value == nullptr)
        return error(DB_ERR_REQUIRE_FAIL);

    LOG_DEBUG("query with sql:{}", sql);
    auto param = static_cast<db_param_query*>(value);
    DEFER(release(DB_PARAM_QUERY, param); param = nullptr;);
    param->db_id = db_id.c_str();
    param->sql = sql.c_str();

    db_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(db_context);
    _query(ctx);
    if (param->result != DB_OK)
        return error(param->result);

    for (size_t i = 0; i < DB_MAX_QUERY_OUTPUT_NUM_LVL1; i++)
    {
        std::vector<std::string> row;
        for (size_t j = 0; j < DB_MAX_QUERY_OUTPUT_NUM_LVL2; j++)
        {
            if (param == nullptr || param->out == nullptr || param->out_len == nullptr)
                continue;

            if (param->out_len[i] == nullptr || *param->out_len[i] == 0)
                continue;

            if (param->out_len[i][j] == nullptr || *param->out_len[i][j] == 0)
                continue;

            std::string elem(param->out[i][j], *param->out_len[i][j]);
            row.emplace_back(elem);
        }

        if (!row.empty())
            outs.emplace_back(row);
    }
    return err_t();
}

err_t db_core::query(
    std::vector<std::vector<std::string>>& outs,
    const std::string& db_id,
    const std::string& tbl,
    const std::vector<std::string>& contents,
    const int count)
{
    std::string sql;
    std::string limit_sql;
    if (count > 0)
        limit_sql = std::string(" LIMIT ") + std::to_string(count);

    if (db_id == "dict")
    {
        // SELECT password FROM passwords LIMIT 100;
        sql = std::string("SELECT " + fmt_strs(contents) + " FROM " + tbl + limit_sql + ";");
        return query(outs, db_id, sql);
    }
    else
    {
        return error(DB_ERR_DB_NOT_EXIST);
    }
}