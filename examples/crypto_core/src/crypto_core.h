#ifndef CRYPTO_CORE_H
#define CRYPTO_CORE_H

#include <hj/os/dll.h>

// ---------------- error code -----------------
#ifndef CRYPTO_ERR_START
#define CRYPTO_ERR_START 0
#endif

#ifndef OK
#define OK                                         (CRYPTO_ERR_START)
#endif

#define CRYPTO_ERR_FAIL                            (CRYPTO_ERR_START + 1)
#define CRYPTO_ERR_CORE_NOT_LOAD                   (CRYPTO_ERR_START + 2)
#define CRYPTO_ERR_CORE_LOAD_FAIL                  (CRYPTO_ERR_START + 3)
#define CRYPTO_ERR_VERSION_MISMATCH                (CRYPTO_ERR_START + 4)
#define CRYPTO_ERR_INIT_FAIL                       (CRYPTO_ERR_START + 5)
#define CRYPTO_ERR_QUIT_FAIL                       (CRYPTO_ERR_START + 6)

#define CRYPTO_ERR_INVALID_FMT                     (CRYPTO_ERR_START + 101)
#define CRYPTO_ERR_INVALID_INPUT                   (CRYPTO_ERR_START + 102)
#define CRYPTO_ERR_INVALID_OUTPUT                  (CRYPTO_ERR_START + 103)
#define CRYPTO_ERR_INVALID_ALGO                    (CRYPTO_ERR_START + 104)
#define CRYPTO_ERR_INVALID_KEY                     (CRYPTO_ERR_START + 105)
#define CRYPTO_ERR_INVALID_PADDING                 (CRYPTO_ERR_START + 106)
#define CRYPTO_ERR_INVALID_IV                      (CRYPTO_ERR_START + 107)
#define CRYPTO_ERR_INVALID_MODE                    (CRYPTO_ERR_START + 108)

#define CRYPTO_ERR_ENCRYPT_AES_FAILED              (CRYPTO_ERR_START + 201)
#define CRYPTO_ERR_ENCRYPT_BASE64_FAILED           (CRYPTO_ERR_START + 202)
#define CRYPTO_ERR_ENCRYPT_DES_FAILED              (CRYPTO_ERR_START + 203)
#define CRYPTO_ERR_ENCRYPT_MD5_FAILED              (CRYPTO_ERR_START + 204)
#define CRYPTO_ERR_ENCRYPT_SHA256_FAILED           (CRYPTO_ERR_START + 205)
#define CRYPTO_ERR_ENCRYPT_RSA_FAILED              (CRYPTO_ERR_START + 206)

#define CRYPTO_ERR_DECRYPT_AES_FAILED              (CRYPTO_ERR_START + 301)
#define CRYPTO_ERR_DECRYPT_BASE64_FAILED           (CRYPTO_ERR_START + 302)
#define CRYPTO_ERR_DECRYPT_DES_FAILED              (CRYPTO_ERR_START + 303)
#define CRYPTO_ERR_DECRYPT_MD5_FAILED              (CRYPTO_ERR_START + 304)
#define CRYPTO_ERR_DECRYPT_SHA256_FAILED           (CRYPTO_ERR_START + 305)
#define CRYPTO_ERR_DECRYPT_RSA_FAILED              (CRYPTO_ERR_START + 306)

#define CRYPTO_ERR_KEYGEN_FAIL                     (CRYPTO_ERR_START + 401)

#define CRYPTO_ERR_FORMAT_FAIL                     (CRYPTO_ERR_START + 501)
#define CRYPTO_ERR_FORMAT_STYLE_NOT_FOUND          (CRYPTO_ERR_START + 502)
#define CRYPTO_ERR_FORMAT_INVALID_TARGET           (CRYPTO_ERR_START + 503)
#define CRYPTO_ERR_FORMAT_HEX_FAILED               (CRYPTO_ERR_START + 504)
#define CRYPTO_ERR_FORMAT_BASE64_FAILED            (CRYPTO_ERR_START + 505)

#define CRYPTO_ERR_UNFORMAT_FAIL                   (CRYPTO_ERR_START + 601)
#define CRYPTO_ERR_UNFORMAT_STYLE_NOT_FOUND        (CRYPTO_ERR_START + 602)
#define CRYPTO_ERR_UNFORMAT_INVALID_TARGET         (CRYPTO_ERR_START + 603)
#define CRYPTO_ERR_UNFORMAT_HEX_FAILED             (CRYPTO_ERR_START + 604)
#define CRYPTO_ERR_UNFORMAT_BASE64_FAILED          (CRYPTO_ERR_START + 605)

#define CRYPTO_ERR_REQUIRE_FAIL                    (CRYPTO_ERR_START + 701)
#define CRYPTO_ERR_RELEASE_FAIL                    (CRYPTO_ERR_START + 702)

#ifndef CRYPTO_ERR_END
#define CRYPTO_ERR_END                             (CRYPTO_ERR_START + 999)
#endif

// --------------- const --------------------
#define CRYPTO_MAX_OUTPUT_NUM 1024   // 16KB * 1024 = 16MB
#define CRYPTO_MAX_OUTPUT_SIZE 16384 // 16KB

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
C_STYLE_EXPORT void crypto_version(sdk_context ctx);

C_STYLE_EXPORT void crypto_init(sdk_context ctx);

C_STYLE_EXPORT void crypto_quit(sdk_context ctx);

C_STYLE_EXPORT void crypto_require(sdk_context ctx);

C_STYLE_EXPORT void crypto_release(sdk_context ctx);

C_STYLE_EXPORT void crypto_encrypt(sdk_context ctx);

C_STYLE_EXPORT void crypto_decrypt(sdk_context ctx);

C_STYLE_EXPORT void crypto_keygen(sdk_context ctx);

#endif