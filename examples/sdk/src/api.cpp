#include "api.h"

#include <hj/util/once.hpp>

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