#include "core.h"
#include "comm.h"
#include <libcpp/util/defer.hpp>

crypto_core::crypto_core()
{
#if defined(_WIN32)
    _dll = dll_open("crypto_core.dll", DLL_RTLD_NOW);
#else
    _dll = dll_open("libcrypto_core.so", DLL_RTLD_NOW);
#endif
    if (_dll == nullptr)
    {
        LOG_ERROR("load core dll fail");
        return;
    }

    _version = static_cast<sdk_api>(dll_get(_dll, "version"));
    _init = static_cast<sdk_api>(dll_get(_dll, "init"));
    _quit = static_cast<sdk_api>(dll_get(_dll, "quit"));
    _encrypt = static_cast<sdk_api>(dll_get(_dll, "encrypt"));
    _decrypt = static_cast<sdk_api>(dll_get(_dll, "decrypt"));
}

crypto_core::~crypto_core()
{
    quit();

    _version = nullptr;
    _init = nullptr;
    _quit = nullptr;
    _encrypt = nullptr;
    _decrypt = nullptr;
    if (_dll)
    {
#if defined(_WIN32)
        dll_close(_dll);
#else
        dll_close(_dll);
#endif
        _dll = nullptr;
    }
}

err_t crypto_core::version(int& major, int& minor, int& patch)
{
    if (_version == nullptr)
        return error(ERR_CORE_NOT_LOAD);

    context ctx;
    auto param = new param_version{};
    DEFER(delete param; param = nullptr;);

    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(context);
    _version(ctx);
    major = param->major;
    minor = param->minor;
    patch = param->patch;
    return err_t();
}

err_t crypto_core::init()
{
    if (_init == nullptr)
        return error(ERR_CORE_NOT_LOAD);

    context ctx;
    auto param = new param_init{};
    DEFER(delete param; param = nullptr;);

    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(context);
    _init(ctx);
    if (param->result != OK)
    {
        LOG_ERROR("core init failed");
        return error(param->result);
    }

    return err_t();
}

err_t crypto_core::quit()
{
    if (_quit == nullptr)
        return error(ERR_CORE_NOT_LOAD);

    context ctx;
    auto param = new param_quit{};
    DEFER(delete param; param = nullptr;);
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(context);
    _quit(ctx);
    if (param->result != OK)
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
    char buf[OUTPUT_BUF];
    size_t buf_len = 0;
    if (!out.empty())
    {
        strncpy_s(buf, out.size(), out.c_str(), buf_len);
        buf_len = out.size();
    }

    context ctx;
    auto param = make_param_encrypt();
    DEFER(delete param; param = nullptr;);
    param->result = FAIL;
    param->out = buf;
    param->out_len = &buf_len;
    param->in = in.c_str();
    param->content = content.c_str();
    param->algo = algo.c_str();
    param->mode = mode.c_str();
    param->key = key.c_str();
    param->padding = padding.c_str();
    param->iv = iv.c_str();
    param->fmt = fmt.c_str();
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(context);
    _encrypt(ctx);
    if (param->result != OK)
        return error(param->result);

    LOG_DEBUG("encrypt success, out_len: {}, buf: {}", buf_len, std::string(buf, buf_len));
    out.assign(buf, buf_len);
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
    char buf[OUTPUT_BUF];
    size_t buf_len = 0;
    if (!out.empty())
    {
        strncpy_s(buf, out.size(), out.c_str(), buf_len);
        buf_len = out.size();
    }

    context ctx;
    auto param = make_param_decrypt();
    DEFER(delete param; param = nullptr;);
    param->result = FAIL;
    param->out = buf;
    param->out_len = &buf_len;
    param->in = in.c_str();
    param->content = content.c_str();
    param->algo = algo.c_str();
    param->mode = mode.c_str();
    param->key = key.c_str();
    param->passwd = passwd.c_str();
    param->padding = padding.c_str();
    param->iv = iv.c_str();
    param->fmt = fmt.c_str();
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(context);
    _decrypt(ctx);
    if (param->result != OK)
        return error(param->result);

    LOG_DEBUG("decrypt success, out_len: {}, buf: {}", buf_len, std::string(buf, buf_len));
    out.assign(buf, buf_len);
    return err_t();
}