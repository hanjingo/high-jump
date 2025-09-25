#include "core.h"

#include <hj/util/defer.hpp>

// ------------------------------ lic core ------------------------------
lic_core::lic_core()
{
    auto err = load();
    if (err)
        throw std::runtime_error("lic_core load fail with err: " + err.message());
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
    if (_dll == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    _version = static_cast<sdk_api>(dll_get(_dll, "lic_version"));
    _init = static_cast<sdk_api>(dll_get(_dll, "lic_init"));
    _quit = static_cast<sdk_api>(dll_get(_dll, "lic_quit"));
    return err_t();
}

err_t lic_core::unload()
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

err_t lic_core::reload()
{
    auto err = unload();
    if (err)
        return err;

    return load();
}

err_t lic_core::version(int& major, int& minor, int& patch)
{
    if (_version == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_version{};
    DEFER(delete param; param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(sdk_context);
    _version(ctx);
    major = param->major;
    minor = param->minor;
    patch = param->patch;
    return err_t();
}

err_t lic_core::init()
{
    if (_init == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_init{};
    DEFER(delete param; param = nullptr;);

    param->result = ERR_FAIL;

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(sdk_context);
    _init(ctx);
    if (param->result != OK)
        return error(param->result);

    return err_t();
}

err_t lic_core::quit()
{
    if (_quit == nullptr)
        return error(ERR_LIC_CORE_LOAD_FAIL);

    auto param = new lic_param_quit{};
    DEFER(delete param; param = nullptr;);

    sdk_context ctx;
    ctx.user_data = param;
    ctx.cb = nullptr;
    ctx.sz = sizeof(sdk_context);
    _quit(ctx);
    if (param->result != OK)
        return error(param->result);

    return err_t();
}