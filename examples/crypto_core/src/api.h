#ifndef CRYPTO_CORE_API_H
#define CRYPTO_CORE_API_H

#include <libcpp/os/dll.h>

typedef struct crypto_context
{
    unsigned long sz;
    void* user_data;
    void (*cb)(void*);
} crypto_context;

typedef void(*crypto_callback)(void*);
typedef void(*crypto_api)(crypto_context);

C_STYLE_EXPORT void crypto_version(crypto_context ctx);

C_STYLE_EXPORT void crypto_init(crypto_context ctx);

C_STYLE_EXPORT void crypto_quit(crypto_context ctx);

// add your code here...
// ---------------- error code [0, 999] -----------------
#define CRYPTO_OK                                  0
#define CRYPTO_ERR_FAIL                            1
#define CRYPTO_ERR_CORE_NOT_LOAD                   2
#define CRYPTO_ERR_CORE_LOAD_FAIL                  3
#define CRYPTO_ERR_VERSION_MISMATCH                4
#define CRYPTO_ERR_INIT_FAIL                       5
#define CRYPTO_ERR_QUIT_FAIL                       6

#define CRYPTO_ERR_INVALID_FMT                     100
#define CRYPTO_ERR_INVALID_INPUT                   101
#define CRYPTO_ERR_INVALID_OUTPUT                  102
#define CRYPTO_ERR_INVALID_ALGO                    103
#define CRYPTO_ERR_INVALID_KEY                     104
#define CRYPTO_ERR_INVALID_PADDING                 105
#define CRYPTO_ERR_INVALID_IV                      106
#define CRYPTO_ERR_INVALID_MODE                    107

#define CRYPTO_ERR_ENCRYPT_AES_FAILED              200
#define CRYPTO_ERR_ENCRYPT_BASE64_FAILED           201
#define CRYPTO_ERR_ENCRYPT_DES_FAILED              202
#define CRYPTO_ERR_ENCRYPT_MD5_FAILED              203
#define CRYPTO_ERR_ENCRYPT_SHA256_FAILED           204
#define CRYPTO_ERR_ENCRYPT_RSA_FAILED              205

#define CRYPTO_ERR_DECRYPT_AES_FAILED              300
#define CRYPTO_ERR_DECRYPT_BASE64_FAILED           301
#define CRYPTO_ERR_DECRYPT_DES_FAILED              302
#define CRYPTO_ERR_DECRYPT_MD5_FAILED              303
#define CRYPTO_ERR_DECRYPT_SHA256_FAILED           304
#define CRYPTO_ERR_DECRYPT_RSA_FAILED              305

#define CRYPTO_ERR_KEYGEN_FAIL                     400

#define CRYPTO_ERR_FORMAT_FAIL                     500
#define CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND          501
#define CRYPTO_ERR_FORMAT_INVALID_TARGET           502
#define CRYPTO_ERR_FORMAT_HEX_FAILED               503
#define CRYPTO_ERR_FORMAT_BASE64_FAILED            504

#define CRYPTO_ERR_UNFORMAT_FAIL                   600
#define CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND        601
#define CRYPTO_ERR_UNFORMAT_INVALID_TARGET         602
#define CRYPTO_ERR_UNFORMAT_HEX_FAILED             603
#define CRYPTO_ERR_UNFORMAT_BASE64_FAILED          604

#define CRYPTO_ERR_REQUIRE_FAIL                    700
#define CRYPTO_ERR_RELEASE_FAIL                    701

#define CRYPTO_ERR_END 999

// --------------- const --------------------
#define CRYPTO_OUTPUT_BUF 16384 // 16KB

#define CRYPTO_MAX_KEY_NUM 2
#define CRYPTO_MAX_KEY_SIZE 8192 // 8KB

#define CRYPTO_MAX_ALGO_LEN 16
#define CRYPTO_MAX_FMT_LEN  16
#define CRYPTO_MAX_MODE_LEN 16

#define CRYPTO_MAX_DATA_POOL_SIZE 1024

// --------------- data struct --------------------
#define CRYPTO_PARAM_ENCRYPT 4
#define CRYPTO_PARAM_DECRYPT 5
#define CRYPTO_PARAM_KEYGEN  6

typedef struct crypto_param_require
{
    int type;
    void* value;
} crypto_param_require;

typedef struct crypto_param_release
{
    int type;
    void* value;
} crypto_param_release;

typedef struct crypto_param_version
{
    int major;
    int minor;
    int patch;
} crypto_param_version;

typedef struct crypto_param_init
{
    size_t data_pool_size;

    int result;
} crypto_param_init;

typedef struct crypto_param_quit
{
    int result;
} crypto_param_quit;

typedef struct crypto_param_encrypt
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
} crypto_param_encrypt;

typedef struct crypto_param_decrypt
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
} crypto_param_decrypt;

typedef struct crypto_param_keygen
{
    char** out;
    size_t** out_len;
    const char* algo;
    const char* fmt;
    const char* mode;
    size_t bits;

    int result;
} crypto_param_keygen;

typedef struct crypto_param_guess
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
} crypto_param_guess;

// --------------- API --------------------
C_STYLE_EXPORT void crypto_require(crypto_context ctx);

C_STYLE_EXPORT void crypto_release(crypto_context ctx);

C_STYLE_EXPORT void crypto_encrypt(crypto_context ctx);

C_STYLE_EXPORT void crypto_decrypt(crypto_context ctx);

C_STYLE_EXPORT void crypto_keygen(crypto_context ctx);

#endif