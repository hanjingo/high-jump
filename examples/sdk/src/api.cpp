#include "api.h"

#include <libcpp/testing/crash.hpp>
#include <libcpp/util/once.hpp>

C_STYLE_EXPORT void version(context ctx)
{
    if (sizeof(ctx) != ctx.sz)
        return;

    // add your code here ...
}

C_STYLE_EXPORT void init(context ctx)
{
    ONCE(
        if (sizeof(ctx) != ctx.sz)
            return;

        // add crash handle support
        libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
        libcpp::crash_handler::instance()->set_local_path("./");

        // add your code here ...
    )
}

C_STYLE_EXPORT void quit(context ctx)
{
    ONCE(
        if (sizeof(ctx) != ctx.sz)
            return;

        // add your code here ...
    )
}