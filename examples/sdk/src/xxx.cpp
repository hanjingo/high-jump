#include "xxx.h"

C_STYLE_EXPORT void xxx(sdk_context *ctx)
{
    if(ctx == nullptr || ctx->sz < sizeof(sdk_context))
        return;

    if(ctx->user_data == nullptr)
        return;

    // add your code here ...
}