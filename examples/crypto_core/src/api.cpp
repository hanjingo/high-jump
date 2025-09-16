#include "api.h"

#include <libcpp/util/init.hpp>
#include <libcpp/os/env.h>
#include <libcpp/testing/crash.hpp>

#include "comm.h"
#include "encrypt.h"
#include "decrypt.h"
#include "keygen.h"

C_STYLE_EXPORT void version(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<param_version*>(ctx.user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;
    if (ctx.cb == NULL)
		return;

	ctx.cb((void*)version);
}

C_STYLE_EXPORT void init(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add crash handle support
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path("./");

    // add log support
    libcpp::logger::instance()->set_level(libcpp::log_lvl::debug);

    // add your code here ...
    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<param_init*>(ctx.user_data);
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

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void quit(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<param_quit*>(ctx.user_data);
    ret->result = OK;
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

// add your code here...
C_STYLE_EXPORT void encrypt(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<param_encrypt*>(ctx.user_data);
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

        strncpy_s(ret->out, OUTPUT_BUF, out.c_str(), out.size());
    }
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void decrypt(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<param_decrypt*>(ctx.user_data);
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

        strncpy_s(ret->out, OUTPUT_BUF, out.c_str(), out.size());
    }
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}