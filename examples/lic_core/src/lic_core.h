#ifndef LIC_CORE_API_H
#define LIC_CORE_API_H

#include <hj/os/dll.h>

// ---------------- error code -----------------
#ifndef LIC_ERR_START
#define LIC_ERR_START 0
#endif

#ifndef OK
#define OK                                      (LIC_ERR_START)
#endif

#define LIC_ERR_FAIL                            (LIC_ERR_START + 1)
#define LIC_ERR_INIT_FAIL                       (LIC_ERR_START + 2)

#ifndef LIC_ERR_END
#define LIC_ERR_END                             (LIC_ERR_START + 999)
#endif

// --------------- data struct --------------------
typedef struct lic_param_version
{
    int major;
    int minor;
    int patch;
} lic_param_version;

typedef struct lic_param_init
{
    int result;
} lic_param_init;

typedef struct lic_param_quit
{
    int result;
} lic_param_quit;

#define LIC_PARAM_GENERATE 1
typedef struct lic_param_generate
{
    const char* out;

    int result;
} lic_param_generate;

// --------------- API --------------------
C_STYLE_EXPORT void lic_version(sdk_context ctx);

C_STYLE_EXPORT void lic_init(sdk_context ctx);

C_STYLE_EXPORT void lic_quit(sdk_context ctx);

#endif // LIC_CORE_API_H