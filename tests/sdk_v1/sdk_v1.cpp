#include "sdk_v1.h"
#include <iostream>

SDK_API void hello(sdk_context_t *ctx)
{
    if(ctx == nullptr || ctx->size < sizeof(sdk_context)
       || ctx->version < SDK_V1_API_VERSION)
        return;

    std::cout << "sdk_v1 hello:" << std::endl;
    if(ctx->user_data != NULL)
    {
        hello_param_t *param = (hello_param_t *) ctx->user_data;
        std::cout << "param.num: " << param->num << std::endl;
    }
}

SDK_API void world(sdk_context_t *ctx)
{
    if(ctx == nullptr || ctx->size < sizeof(sdk_context)
       || ctx->version < SDK_V1_API_VERSION)
        return;

    std::cout << "sdk_v1 world:" << std::endl;
    if(ctx->user_data != NULL)
    {
        world_param_t *param = (world_param_t *) ctx->user_data;
        std::cout << "param.in: " << param->in << std::endl;
        std::cout << "param.in_len: " << param->in_len << std::endl;

        param->out = strncpy(param->out, "Hello from sdk_v1!", param->out_len);
        param->out_len = strlen(param->out);

        if(ctx->callback != NULL)
            ctx->callback((void *) param);
    }
}