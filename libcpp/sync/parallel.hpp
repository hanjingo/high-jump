#ifndef PARALLEL_HPP
#define PARALLEL_HPP

#include <functional>

#include <tbb/tbb.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

namespace libcpp
{
template<typename T>
using blocked_range = tbb::blocked_range<T>;

class parallel
{
public:
    template<typename... Args>
    static void for_range(Args&& ... args)
    {
        tbb::parallel_for(std::forward<Args>(args)...);
    }

    // TODO
    // template<typename Begin, typename End, typename... Args>
    // static void for_each(Begin begin, End end, Args&& ... args)
    // {
    //     tbb::parallel_for_each(begin, end, std::forward<Args>(args)...);
    // }

    // static void while() {}

    // static void reduce() {}

    // static void sort() {}

    // static void scan() {}
};

}

#endif