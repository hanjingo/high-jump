#include "lic_core.h"

#include <hj/util/once.hpp>
#include <hj/util/license.hpp>

#include "comm.h"
#include "issuer.h"

#include <iostream>

C_STYLE_EXPORT void lic_version(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<lic_param_version*>(ctx.user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void lic_init(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<lic_param_init*>(ctx.user_data);
        ret->result = OK;
        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

C_STYLE_EXPORT void lic_quit(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<lic_param_quit*>(ctx.user_data);
        ret->result = OK;

        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

C_STYLE_EXPORT void lic_add_issuer(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<lic_param_add_issuer*>(ctx.user_data);
    if (ret->issuer_id == nullptr || ret->algo == nullptr || ret->times < 1)
    {
        ret->result = LIC_ERR_INVALID_PARAM;
        return;
    }

    auto id = std::string(ret->issuer_id);
    auto algo = str_to_sign_algo(std::string(ret->algo));
    auto times = ret->times;
    std::vector<std::string> keys;
    for (size_t i = 0; i < ret->keys_num && i < LIC_MAX_KEY_NUM; i++)
    {
        if (ret->keys[i] != nullptr)
            keys.emplace_back(std::string(ret->keys[i]));
        else
            keys.emplace_back("");
    }

    auto issuer = std::make_unique<hj::license::issuer>(id, algo, keys, times);
    ret->result = issuer_mgr::instance().add(std::move(issuer));
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void lic_issue(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<lic_param_issue*>(ctx.user_data);
    if (ret->out == nullptr || ret->out_len == nullptr)
    {
        ret->result = LIC_ERR_INVALID_PARAM;
        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
        return;
    }

    std::string output;
    bool to_file = false;
    if (*ret->out_len > 0)
    {
        output.assign(ret->out, *ret->out_len);
        to_file = true;
    }

    // parse claims
    std::vector<std::pair<std::string, std::string>> claims;
    for (size_t i = 0; ret->claims_num > 0 && i + 1 < ret->claims_num && i < LIC_MAX_CLAIM_NUM; i += 2)
    {
        if (ret->claims[i] == nullptr || ret->claims[i + 1] == nullptr)
            continue;

        claims.emplace_back(std::make_pair(std::string(ret->claims[i]), std::string(ret->claims[i + 1])));
    }

    std::string licensee = ret->licensee ? std::string(ret->licensee) : "";
    std::string issuer_id = ret->issuer_id ? std::string(ret->issuer_id) : "";
    size_t time = ret->time;

    ret->result = issuer_mgr::instance().issue(output, licensee, issuer_id, time, claims);
    if (!to_file && ret->out != nullptr)
    {
        strncpy_s(ret->out, LIC_MAX_OUTPUT_SIZE, output.c_str(), output.size());
        *ret->out_len = output.size();
    }

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void lic_verify(sdk_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;
}

