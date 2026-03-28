#ifndef LOG_CORE_API_H
#define LOG_CORE_API_H

#include <hj/os/dll.h>
#include <hj/util/once.hpp>

// ---------------- error code [3000, 3999] -----------------
#ifndef OK
#define OK 0
#endif

#define LOG_ERR_START 3000

#define LOG_ERR_FAIL 3001
#define LOG_ERR_INIT_FAIL 3002
#define LOG_ERR_INVALID_PARAM 3003

#define LOG_ERR_END 3999

// --------------- const --------------------

// --------------- data struct --------------------
typedef struct log_param_version
{
    int major;
    int minor;
    int patch;
} log_param_version;

typedef struct log_param_init
{
    int result;
} log_param_init;

typedef struct log_param_quit
{
    int result;
} log_param_quit;

// --------------- API --------------------
C_STYLE_EXPORT void log_version(sdk_context *ctx);

C_STYLE_EXPORT void log_init(sdk_context *ctx);

C_STYLE_EXPORT void log_quit(sdk_context *ctx);

#endif // LOG_CORE_API_H