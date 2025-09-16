#ifndef API_H
#define API_H

#include <libcpp/os/dll.h>

typedef struct context
{
    unsigned long sz;
    void* user_data;
    void (*cb)(void*);
} context;

typedef void(*sdk_callback)(void*);
typedef void(*sdk_api)(context);

C_STYLE_EXPORT void version(context ctx);

C_STYLE_EXPORT void init(context ctx);

C_STYLE_EXPORT void quit(context ctx);

// add your code here...

#endif