#ifndef SDK_V2_H
#define SDK_V2_H

#include <hj/os/sdk.h>

#define SDK_HELLO_PARAM_ABI_VERSION_1 1U
#define SDK_HELLO_PARAM_ABI_VERSION_2 2U

#define SDK_WORLD_PARAM_ABI_VERSION_1 1U
#define SDK_WORLD_PARAM_ABI_VERSION_2 2U

typedef struct hello_param
{
    SDK_ABI_HEADER

    int32_t num;

    const char *tag;
    uint32_t    tag_len;
} hello_param_t;

typedef struct world_param
{
    SDK_ABI_HEADER

    const char *in;
    uint32_t    in_len;

    const char *memo;
    uint32_t    memo_len;

    char    *out;
    uint32_t out_len;
} world_param_t;

SDK_C_API void hello(sdk_context_t *ctx);
SDK_C_API void world(sdk_context_t *ctx);

#endif