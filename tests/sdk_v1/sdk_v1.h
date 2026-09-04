#ifndef SDK_V1_H
#define SDK_V1_H

#include <hj/os/sdk.h>

#define SDK_V1_API_VERSION 1

typedef struct hello_param
{
    int num;
} hello_param_t;

typedef struct world_param
{
    const char *in;
    int         in_len;

    char *out;
    int   out_len;
} world_param_t;

SDK_API void hello(sdk_context_t *ctx);

SDK_API void world(sdk_context_t *ctx);

#endif