#include "crypto_core.h"

#include <string.h>

#include <hj/util/init.hpp>
#include <hj/util/once.hpp>
#include <hj/os/env.h>
#include <hj/testing/crash.hpp>

#include "comm.h"
#include "encrypt.h"
#include "decrypt.h"
#include "keygen.h"
#include "data.h"

C_STYLE_EXPORT void crypto_version(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<crypto_param_version*>(ctx.user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void crypto_init(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        // add crash handle support
        hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
        hj::crash_handler::instance()->set_local_path("./");

        // add log support
        hj::logger::instance()->set_level(hj::log_lvl::debug);

        // add your code here ...
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<crypto_param_init*>(ctx.user_data);
        ret->result = OK;

        encryptor_mgr::instance().add(std::make_unique<aes_encryptor>());
        encryptor_mgr::instance().add(std::make_unique<base64_encryptor>());
        encryptor_mgr::instance().add(std::make_unique<des_encryptor>());
        encryptor_mgr::instance().add(std::make_unique<md5_encryptor>());
        encryptor_mgr::instance().add(std::make_unique<sha256_encryptor>());
        encryptor_mgr::instance().add(std::make_unique<rsa_encryptor>());

        decryptor_mgr::instance().add(std::make_unique<aes_decryptor>());
        decryptor_mgr::instance().add(std::make_unique<base64_decryptor>());
        decryptor_mgr::instance().add(std::make_unique<des_decryptor>());
        decryptor_mgr::instance().add(std::make_unique<rsa_decryptor>());

        keymaker_mgr::instance().add(std::make_unique<rsa_keymaker>());

        data_mgr::instance().init(ret->data_pool_size);

        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

C_STYLE_EXPORT void crypto_quit(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
    ONCE(
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<crypto_param_quit*>(ctx.user_data);
        ret->result = OK;

        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

// add your code here...
C_STYLE_EXPORT void crypto_require(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;
     
    auto ret = static_cast<crypto_param_require*>(ctx.user_data);
    ret->value = data_mgr::instance().require(ret->type);

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void crypto_release(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;
     
    auto ret = static_cast<crypto_param_release*>(ctx.user_data);
    data_mgr::instance().release(ret->type, ret->value);
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void crypto_encrypt(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<crypto_param_encrypt*>(ctx.user_data);
    int len = (ret->out_len == NULL) ? 0 : *ret->out_len;
    auto out = (ret->out == NULL) ? std::string() : std::string(ret->out, len);
    auto in = (ret->in == NULL) ? std::string() : std::string(ret->in);
    auto content = (ret->content == NULL) ? std::string() : std::string(ret->content);
    auto algo = (ret->algo == NULL) ? std::string() : std::string(ret->algo);
    auto mode = (ret->mode == NULL) ? std::string() : std::string(ret->mode);
    auto key = (ret->key == NULL) ? std::string() : std::string(ret->key);
    auto padding = (ret->padding == NULL) ? std::string() : std::string(ret->padding);
    auto iv = (ret->iv == NULL) ? std::string() : std::string(ret->iv);
    auto fmt = (ret->fmt == NULL) ? std::string() : std::string(ret->fmt);
    ret->result = encryptor_mgr::instance().encrypt(
        out, in, content, algo, mode, key, padding, iv, fmt);
    if (ret->result == OK && ret->out != NULL)
    {
        if (ret->out_len != NULL)
            *ret->out_len = out.size();

        strncpy_s(ret->out, CRYPTO_MAX_OUTPUT_SIZE, out.c_str(), out.size());
    }
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void crypto_decrypt(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<crypto_param_decrypt*>(ctx.user_data);
    int len = (ret->out_len == NULL) ? 0 : *ret->out_len;
    auto out = (ret->out == NULL) ? std::string() : std::string(ret->out, len);
    auto in = (ret->in == NULL) ? std::string() : std::string(ret->in);
    auto content = (ret->content == NULL) ? std::string() : std::string(ret->content);
    auto algo = (ret->algo == NULL) ? std::string() : std::string(ret->algo);
    auto mode = (ret->mode == NULL) ? std::string() : std::string(ret->mode);
    auto key = (ret->key == NULL) ? std::string() : std::string(ret->key);
    auto passwd = (ret->passwd == NULL) ? std::string() : std::string(ret->passwd);
    auto padding = (ret->padding == NULL) ? std::string() : std::string(ret->padding);
    auto iv = (ret->iv == NULL) ? std::string() : std::string(ret->iv);
    auto fmt = (ret->fmt == NULL) ? std::string() : std::string(ret->fmt);
    ret->result = decryptor_mgr::instance().decrypt(
        out, in, content, algo, mode, key, passwd, padding, iv, fmt);
    if (ret->result == OK && ret->out != NULL)
    {
        if (ret->out_len != NULL)
            *ret->out_len = out.size();

        strncpy_s(ret->out, CRYPTO_MAX_OUTPUT_SIZE, out.c_str(), out.size());
    }
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void crypto_keygen(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<crypto_param_keygen*>(ctx.user_data);
    auto algo = (ret->algo == NULL) ? std::string() : std::string(ret->algo);
    auto fmt = (ret->fmt == NULL) ? std::string() : std::string(ret->fmt);
    auto mode = (ret->mode == NULL) ? std::string() : std::string(ret->mode);
    auto bits = ret->bits;

    std::vector<std::string> keys;
    ret->result = keymaker_mgr::instance().make(keys, algo, fmt, mode, bits);
    for (auto e : keys)
        LOG_DEBUG("key:{}", e);

    if (ret->result == OK)
    {
        for (size_t i = 0; i < keys.size() && i < CRYPTO_MAX_KEY_NUM; i++)
        {
            if (ret->out == NULL || ret->out_len == NULL)
                break;

            if (ret->out[i] == NULL || ret->out_len[i] == NULL)
                continue;

            strncpy_s(ret->out[i], CRYPTO_MAX_KEY_SIZE, keys[i].c_str(), keys[i].size());
            *ret->out_len[i] = keys[i].size();
        }
    }

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}