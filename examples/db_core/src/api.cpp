#include "api.h"

#include <iostream>
#include <libcpp/util/once.hpp>
#include <libcpp/encoding/ini.hpp>

#include "comm.h"
#include "db_mgr.h"
#include "data_mgr.h"

C_STYLE_EXPORT void db_version(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<db_param_version*>(ctx.user_data);
    ret->major = MAJOR_VERSION;
    ret->minor = MINOR_VERSION;
    ret->patch = PATCH_VERSION;

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void db_init(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        // add your code here ...
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<db_param_init*>(ctx.user_data);
        if (ret->option == NULL || strlen(ret->option) == 0)
        {
            ret->result = DB_ERR_PARSE_INIT_OPTION_FAIL;
            if (ctx.cb != NULL)
                ctx.cb(static_cast<void*>(ret));

            return;
        }

        // parse config
        libcpp::ini cfg = libcpp::ini::parse(ret->option);
        for (auto& child : cfg) 
        {
            auto& section = child.second;
            auto typ = section.get<std::string>("type", "");
            // init data mgr
            if (typ == "data_pool")
            {
                auto data_pool_size = section.get<size_t>("data_pool_size", 1);
                data_mgr::instance().init(data_pool_size);
            }
            // init sqlite db
            else if(typ == "sqlite")
            {
                std::string db_id = section.get<std::string>("db_id", "");
                std::string db_path = section.get<std::string>("db_path", "");
                size_t conn_capa = section.get<size_t>("db_conn_capa", 0);
                if (db_id.empty() || db_path.empty() || conn_capa == 0)
                    continue;

                db_mgr::instance().add(std::make_unique<sqlite>(
                    db_id.c_str(),
                    db_path.c_str(), 
                    conn_capa));
            }
            // init more ...
        }

        ret->result = DB_OK;
        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

C_STYLE_EXPORT void db_quit(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    ONCE(
        // add your code here ...
        if (ctx.user_data == NULL)
            return;

        auto ret = static_cast<db_param_quit*>(ctx.user_data);
        ret->result = DB_OK;

        if (ctx.cb != NULL)
            ctx.cb(static_cast<void*>(ret));
    )
}

// add your code here...
C_STYLE_EXPORT void db_require(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;
     
    auto ret = static_cast<db_param_require*>(ctx.user_data);
    ret->value = data_mgr::instance().require(ret->type);

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void db_release(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;
     
    auto ret = static_cast<db_param_release*>(ctx.user_data);
    data_mgr::instance().release(ret->type, ret->value);
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void db_exec(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto ret = static_cast<db_param_exec*>(ctx.user_data);
    ret->result = db_mgr::instance().exec(ret->db_id, ret->sql);
    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(ret));
}

C_STYLE_EXPORT void db_query(db_context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    if (ctx.user_data == NULL)
        return;

    auto param = static_cast<db_param_query*>(ctx.user_data);
    std::vector<std::vector<std::string>> outs;
    param->result = db_mgr::instance().query(outs, param->db_id, param->sql);
    if (param->result == DB_OK)
    {
        for (int i = 0; i < outs.size(); i++)
        {
            for (int j = 0; j < outs[i].size(); j++) 
            {
                if (param->out == nullptr || param->out_len == nullptr)
                    continue;

                if (param->out[i] == nullptr || param->out_len[i] == nullptr)
                    continue;

                if (param->out[i][j] == nullptr || param->out_len[i][j] == nullptr)
                    continue;

                strncpy_s(param->out[i][j], DB_MAX_OUTPUT_SIZE, outs[i][j].c_str(), outs[i][j].size());
                *(param->out_len[i][j]) = outs[i][j].size();
            }
        }
    }

    if (ctx.cb != NULL)
        ctx.cb(static_cast<void*>(param));
}