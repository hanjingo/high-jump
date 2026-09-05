#include "sdk_v2.h"
#include <iostream>
#include <cstring>

SDK_C_API void hello(sdk_context_t *ctx)
{
    if(!SDK_VALIDATE_ABI(ctx, sdk_context_t, SDK_CONTEXT_ABI_VERSION_1)
       || !SDK_HAS_FIELD(ctx, sdk_context_t, user_data))
        return;

    if(ctx->user_data != nullptr)
    {
        auto *param = static_cast<hello_param_t *>(ctx->user_data);
        if(!SDK_VALIDATE_ABI(param,
                             hello_param_t,
                             SDK_HELLO_PARAM_ABI_VERSION_2))
            return;

        std::cout << "sdk_v2 hello:" << std::endl;
        if(SDK_HAS_FIELD(param, hello_param_t, num))
        {
            std::cout << "  param.num: " << param->num << std::endl;
        }

        if(param->abi_version >= SDK_HELLO_PARAM_ABI_VERSION_2
           && SDK_HAS_FIELD(param, hello_param_t, tag_len)
           && param->tag != nullptr)
        {
            std::cout << "  param.tag: " << param->tag << std::endl;
            std::cout << "  param.tag_len: " << param->tag_len << std::endl;
        }
    }
}

SDK_C_API void world(sdk_context_t *ctx)
{
    if(!SDK_VALIDATE_ABI(ctx, sdk_context_t, SDK_CONTEXT_ABI_VERSION_1)
       || !SDK_HAS_FIELD(ctx, sdk_context_t, user_data))
        return;

    if(ctx->user_data != nullptr)
    {
        auto *param = static_cast<world_param_t *>(ctx->user_data);

        if(!SDK_VALIDATE_ABI(param,
                             world_param_t,
                             SDK_WORLD_PARAM_ABI_VERSION_2))
            return;

        std::cout << "sdk_v2 world:" << std::endl;
        if(SDK_HAS_FIELD(param, world_param_t, in_len) && param->in != nullptr)
        {
            std::cout << "  param.in: " << param->in << std::endl;
        }

        if(param->abi_version >= SDK_WORLD_PARAM_ABI_VERSION_2
           && SDK_HAS_FIELD(param, world_param_t, memo_len)
           && param->memo != nullptr)
        {
            std::cout << "  param.memo: " << param->memo << std::endl;
        }

        if(SDK_HAS_FIELD(param, world_param_t, out_len) && param->out != nullptr
           && param->out_len > 0)
        {
            strncpy(param->out, "Hello from sdk_v2!", param->out_len - 1);
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