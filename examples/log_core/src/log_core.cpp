#include "log_core.h"

C_STYLE_EXPORT void log_version(sdk_context *ctx)
{
    if(ctx == nullptr || ctx->sz < sizeof(sdk_context))
        return;

    if(ctx->user_data == NULL)
        return;

    auto ret   = static_cast<log_param_version *>(ctx->user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;

    if(ctx->cb != NULL)
        ctx->cb(static_cast<void *>(ret));
}

C_STYLE_EXPORT void log_init(sdk_context *ctx)
{
    if(ctx == nullptr || ctx->sz < sizeof(sdk_context))
        return;

    ONCE(if(ctx->user_data == NULL) return;

         auto ret    = static_cast<log_param_init *>(ctx->user_data);
         ret->result = OK;
         if(ctx->cb != NULL) ctx->cb(static_cast<void *>(ret));)
}

C_STYLE_EXPORT void log_quit(sdk_context *ctx)
{
    if(ctx == nullptr || ctx->sz < sizeof(sdk_context))
        return;

    ONCE(if(ctx->user_data == NULL) return;

         auto ret    = static_cast<log_param_quit *>(ctx->user_data);
         ret->result = OK;

         if(ctx->cb != NULL) ctx->cb(static_cast<void *>(ret));)
}