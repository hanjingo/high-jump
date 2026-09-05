#include "sdk_v1.h"
#include <iostream>
#include <cstring>

SDK_LOCAL bool precheck(sdk_context_t *ctx)
{
    return SDK_HAS_ABI_HEADER(ctx)
           && SDK_HAS_FIELD(ctx, sdk_context_t, user_data);
}

SDK_C_API void hello(sdk_context_t *ctx)
{
    if(!precheck(ctx))
        return;

    if(SDK_HAS_FIELD(ctx, sdk_context_t, user_data)
       && ctx->user_data != nullptr)
    {
        auto *param = static_cast<hello_param_t *>(ctx->user_data);
        if(!SDK_HAS_ABI_HEADER(param))
            return;

        std::cout << "sdk_v1 hello:" << std::endl;
        if(SDK_HAS_FIELD(param, hello_param_t, num))
        {
            std::cout << "  param.num: " << param->num << std::endl;
        }
    }
}

SDK_C_API void world(sdk_context_t *ctx)
{
    if(!precheck(ctx))
        return;

    if(ctx->user_data != nullptr)
    {
        auto *param = static_cast<world_param_t *>(ctx->user_data);

        if(!SDK_HAS_ABI_HEADER(param))
            return;

        std::cout << "sdk_v1 world:" << std::endl;
        if(SDK_HAS_FIELD(param, world_param_t, in_len) && param->in != nullptr)
        {
            std::cout << "  param.in: " << param->in << std::endl;
        }

        if(SDK_HAS_FIELD(param, world_param_t, out_len) && param->out != nullptr
           && param->out_len > 0)
        {
            strncpy(param->out, "Hello from sdk_v1!", param->out_len - 1);
            param->out[param->out_len - 1] = '\0';
            param->out_len = static_cast<uint32_t>(strlen(param->out));
        }

        if(SDK_HAS_FIELD(ctx, sdk_context_t, callback)
           && ctx->callback != nullptr)
        {
            ctx->callback(ctx->user_data);
        }
    }
}