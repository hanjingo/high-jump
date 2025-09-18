#include "api.h"

#include <iostream>
#include <libcpp/util/once.hpp>

#include "comm.h"
#include "db_mgr.h"
#include "sqlite.h"

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

        // add sqlite
        db_mgr::instance().add(std::make_unique<sqlite>(
            ret->sqlite_db_path, 
            ret->sqlite_conn_capa));

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

C_STYLE_EXPORT void db_exec(db_context ctx)
{
    
}