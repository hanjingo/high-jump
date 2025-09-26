#ifndef LIC_CORE_API_H
#define LIC_CORE_API_H

#include <hj/os/dll.h>

// ---------------- error code [3000, 3999] -----------------
#ifndef OK
#define OK 0
#endif

#define LIC_ERR_START                           3000

#define LIC_ERR_FAIL                            3001
#define LIC_ERR_INIT_FAIL                       3002
#define LIC_ERR_INVALID_PARAM                   3003

#define LIC_ERR_ISSUER_EXISTED                  3101
#define LIC_ERR_ISSUER_NOT_EXIST                3102

#define LIC_ERR_INVALID_TIMES                   3201
#define LIC_ERR_CLAIM_MISMATCH                  3202
#define LIC_ERR_KEYS_NOT_CHANGED                3203
#define LIC_ERR_KEYS_NOT_ENOUGH                 3204
#define LIC_ERR_FILE_NOT_EXIST                  3205
#define LIC_ERR_INPUT_STREAM_INVALID            3206
#define LIC_ERR_OUTPUT_STREAM_INVALID           3207

#define LIC_ERR_END                             3999

// --------------- const --------------------
#define LIC_MAX_DATA_POOL_SIZE 1

#define LIC_MAX_OUTPUT_SIZE 4096 // 4KB

#define LIC_MAX_CLAIM_NUM  64    // max 64 claims
#define LIC_MAX_CLAIM_SIZE 128   // max 128 bytes for each claim

#define LIC_MAX_KEY_NUM    4     // max 4 keys for each issuer
#define LIC_MAX_KEY_SIZE   4096  // max 4KB len for each key

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

#define LIC_PARAM_ADD_ISSUER 1
typedef struct lic_param_add_issuer
{
    const char* issuer_id;
    const char* algo;
    const char** keys;
    size_t keys_num;
    size_t times;

    int result;
} lic_param_add_issuer;

#define LIC_PARAM_ISSUE 2
typedef struct lic_param_issue
{
    char* out;
    size_t* out_len;
    const char* algo;
    const char* licensee;
    const char* issuer_id;
    size_t time; // days
    const char** claims; // [[xxx,xxx], [xxx,xxx], ...]
    size_t claims_num;

    int result;
} lic_param_issue;

#define LIC_PARAM_VERIFY 3
typedef struct lic_param_verify
{
    const char* in;

    int result;
} lic_param_verify;


// --------------- API --------------------
C_STYLE_EXPORT void lic_version(sdk_context ctx);

C_STYLE_EXPORT void lic_init(sdk_context ctx);

C_STYLE_EXPORT void lic_quit(sdk_context ctx);

C_STYLE_EXPORT void lic_add_issuer(sdk_context ctx);

C_STYLE_EXPORT void lic_issue(sdk_context ctx);

C_STYLE_EXPORT void lic_verify(sdk_context ctx);

#endif // LIC_CORE_API_H