#ifndef SDK_V1_H
#define SDK_V1_H

#include <hj/os/sdk.h>

#define SDK_HELLO_API_VERSION_1 1U

typedef struct hello_param
{
    SDK_ABI_HEADER

    int32_t num;
} hello_param_t;

typedef struct world_param
{
    SDK_ABI_HEADER

    const char *in;
    uint32_t    in_len;

    char    *out;
    uint32_t out_len;
} world_param_t;

SDK_C_API void hello(sdk_context_t *ctx);

SDK_C_API void world(sdk_context_t *ctx);

#endif