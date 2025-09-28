#ifndef CRYPTO_CORE_H
#define CRYPTO_CORE_H

#include <hj/os/dll.h>

// ---------------- error code [1000, 1999] -----------------
#ifndef OK
#define OK 0
#endif

#define CRYPTO_ERR_START 1000
#define CRYPTO_ERR_FAIL 1001
#define CRYPTO_ERR_CORE_NOT_LOAD 1002
#define CRYPTO_ERR_CORE_LOAD_FAIL 1003
#define CRYPTO_ERR_VERSION_MISMATCH 1004
#define CRYPTO_ERR_INIT_FAIL 1005
#define CRYPTO_ERR_QUIT_FAIL 1006

#define CRYPTO_ERR_INVALID_FMT 1101
#define CRYPTO_ERR_INVALID_INPUT 1102
#define CRYPTO_ERR_INVALID_OUTPUT 1103
#define CRYPTO_ERR_INVALID_ALGO 1104
#define CRYPTO_ERR_INVALID_KEY 1105
#define CRYPTO_ERR_INVALID_PADDING 1106
#define CRYPTO_ERR_INVALID_IV 1107
#define CRYPTO_ERR_INVALID_MODE 1108

#define CRYPTO_ERR_ENCRYPT_AES_FAILED 1201
#define CRYPTO_ERR_ENCRYPT_BASE64_FAILED 1202
#define CRYPTO_ERR_ENCRYPT_DES_FAILED 1203
#define CRYPTO_ERR_ENCRYPT_MD5_FAILED 1204
#define CRYPTO_ERR_ENCRYPT_SHA256_FAILED 1205
#define CRYPTO_ERR_ENCRYPT_RSA_FAILED 1206

#define CRYPTO_ERR_DECRYPT_AES_FAILED 1301
#define CRYPTO_ERR_DECRYPT_BASE64_FAILED 1302
#define CRYPTO_ERR_DECRYPT_DES_FAILED 1303
#define CRYPTO_ERR_DECRYPT_MD5_FAILED 1304
#define CRYPTO_ERR_DECRYPT_SHA256_FAILED 1305
#define CRYPTO_ERR_DECRYPT_RSA_FAILED 1306

#define CRYPTO_ERR_KEYGEN_FAIL 1401

#define CRYPTO_ERR_FORMAT_FAIL 1501
#define CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND 1502
#define CRYPTO_ERR_FORMAT_INVALID_TARGET 1503
#define CRYPTO_ERR_FORMAT_HEX_FAILED 1504
#define CRYPTO_ERR_FORMAT_BASE64_FAILED 1505

#define CRYPTO_ERR_UNFORMAT_FAIL 1601
#define CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND 1602
#define CRYPTO_ERR_UNFORMAT_INVALID_TARGET 1603
#define CRYPTO_ERR_UNFORMAT_HEX_FAILED 1604
#define CRYPTO_ERR_UNFORMAT_BASE64_FAILED 1605

#define CRYPTO_ERR_REQUIRE_FAIL 1701
#define CRYPTO_ERR_RELEASE_FAIL 1702

#define CRYPTO_ERR_END 1999

// --------------- const --------------------
#define CRYPTO_MAX_OUTPUT_NUM 1024   // 16KB * 1024 = 16MB
#define CRYPTO_MAX_OUTPUT_SIZE 16384 // 16KB

#define CRYPTO_MAX_KEY_NUM 2
#define CRYPTO_MAX_KEY_SIZE 8192 // 8KB

#define CRYPTO_MAX_ALGO_LEN 16
#define CRYPTO_MAX_FMT_LEN 16
#define CRYPTO_MAX_MODE_LEN 16

#define CRYPTO_MAX_DATA_POOL_SIZE 1024

// --------------- data struct --------------------
#define CRYPTO_PARAM_ENCRYPT 4
#define CRYPTO_PARAM_DECRYPT 5
#define CRYPTO_PARAM_KEYGEN 6

typedef struct crypto_param_require
{
    int   type;
    void *value;
} crypto_param_require;

typedef struct crypto_param_release
{
    int   type;
    void *value;
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
    char       *out;
    size_t     *out_len;
    const char *in;
    const char *content;
    const char *algo;
    const char *mode;
    const char *key;
    const char *padding;
    const char *iv;
    const char *fmt;

    int result;
} crypto_param_encrypt;

typedef struct crypto_param_decrypt
{
    char       *out;
    size_t     *out_len;
    const char *in;
    const char *content;
    const char *algo;
    const char *mode;
    const char *key;
    const char *passwd;
    const char *padding;
    const char *iv;
    const char *fmt;

    int result;
} crypto_param_decrypt;

typedef struct crypto_param_keygen
{
    char      **out;
    size_t    **out_len;
    const char *algo;
    const char *fmt;
    const char *mode;
    size_t      bits;

    int result;
} crypto_param_keygen;

typedef struct crypto_param_guess
{
    char       *out;
    size_t     *out_len;
    const char *in;
    const char *content;
    const char *algo;
    const char *mode;
    const char *key;
    const char *passwd;
    const char *padding;
    const char *iv;
    const char *fmt;

    int result;
} crypto_param_guess;

// --------------- API --------------------
C_STYLE_EXPORT void crypto_version(sdk_context ctx);

C_STYLE_EXPORT void crypto_init(sdk_context ctx);

C_STYLE_EXPORT void crypto_quit(sdk_context ctx);

C_STYLE_EXPORT void crypto_require(sdk_context ctx);

C_STYLE_EXPORT void crypto_release(sdk_context ctx);

C_STYLE_EXPORT void crypto_encrypt(sdk_context ctx);

C_STYLE_EXPORT void crypto_decrypt(sdk_context ctx);

C_STYLE_EXPORT void crypto_keygen(sdk_context ctx);

#endif