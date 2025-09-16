#ifndef SDK_H
#define SDK_H

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
#define FAIL                                -1
#define OK                                  0
#define ERR_CORE_NOT_LOAD                   1
#define ERR_CORE_LOAD_FAIL                  2
#define ERR_VERSION_MISMATCH                3
#define ERR_INIT_FAIL                       4
#define ERR_QUIT_FAIL                       5

#define ERR_INVALID_SUBCMD                  101
#define ERR_INVALID_FMT                     102
#define ERR_INVALID_INPUT                   103
#define ERR_INVALID_OUTPUT                  104
#define ERR_INVALID_ALGO                    105
#define ERR_INVALID_KEY                     106
#define ERR_INVALID_PADDING                 107
#define ERR_INVALID_IV                      108
#define ERR_INVALID_MODE                    109

#define ERR_ENCRYPT_AES_FAILED              200
#define ERR_ENCRYPT_BASE64_FAILED           201
#define ERR_ENCRYPT_DES_FAILED              202
#define ERR_ENCRYPT_MD5_FAILED              203
#define ERR_ENCRYPT_SHA256_FAILED           204
#define ERR_ENCRYPT_RSA_FAILED              205

#define ERR_DECRYPT_AES_FAILED              300
#define ERR_DECRYPT_BASE64_FAILED           301
#define ERR_DECRYPT_DES_FAILED              302
#define ERR_DECRYPT_MD5_FAILED              303
#define ERR_DECRYPT_SHA256_FAILED           304
#define ERR_DECRYPT_RSA_FAILED              305

#define ERR_KEYGEN_FAIL                     400

#define ERR_FORMAT_FAIL                     500
#define ERR_FORMAT_STYLE_NOT_FOUND          501
#define ERR_FORMAT_INVALID_TARGET           502
#define ERR_FORMAT_HEX_FAILED               503
#define ERR_FORMAT_BASE64_FAILED            504

#define ERR_UNFORMAT_FAIL                   600
#define ERR_UNFORMAT_STYLE_NOT_FOUND        601
#define ERR_UNFORMAT_INVALID_TARGET         602
#define ERR_UNFORMAT_HEX_FAILED             603
#define ERR_UNFORMAT_BASE64_FAILED          604

#define ERR_END 999

#define OUTPUT_BUF 16384 // 16KB

// --------------- data struct --------------------
typedef struct param_version
{
    int major;
    int minor;
    int patch;
} param_version;

typedef struct param_init
{
    int result;
} param_init;

typedef struct param_quit
{
    int result;
} param_quit;

typedef struct param_encrypt
{
    char* out;
    size_t* out_len;
    const char* in;
    const char* content;
    const char* algo;
    const char* mode;
    const char* key;
    const char* padding;
    const char* iv;
    const char* fmt;

    int result;
} param_encrypt;

typedef struct param_decrypt
{
    char* out;
    size_t* out_len;
    const char* in;
    const char* content;
    const char* algo;
    const char* mode;
    const char* key;
    const char* passwd;
    const char* padding;
    const char* iv;
    const char* fmt;

    int result;
} param_decrypt;

// --------------- API --------------------
C_STYLE_EXPORT void encrypt(context ctx);

C_STYLE_EXPORT void decrypt(context ctx);

#endif