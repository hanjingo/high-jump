#include "lic_core.h"

#include <hj/util/once.hpp>

C_STYLE_EXPORT void lic_version(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<lic_param_version*>(ctx.user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void lic_init(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<lic_param_init*>(ctx.user_data);
        ret->result = OK;
        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

C_STYLE_EXPORT void lic_quit(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<lic_param_quit*>(ctx.user_data);
        ret->result = OK;

        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}