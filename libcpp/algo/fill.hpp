#ifndef FILL_HPP
#define FILL_HPP

#include <functional>

namespace libcpp
{

template <typename Container, typename Fn>
void fill (Container &ct, const Fn &&fn)
{
    unsigned long idx = 0;
    for (auto &v : ct)
        v = fn (idx++);
}

template <typename Container, typename Fn>
void fill (Container &ct, const Fn &&fn, const unsigned long n)
{
    unsigned long idx = 0;
    for (auto &v : ct) {
        v = fn (idx++);
        if (idx >= n)
            return;
    }
}

template <typename Iterator, typename Fn>
void fill (Iterator begin, Iterator end, const Fn &&fn)
{
    unsigned long idx = 0;
    for (auto itr = begin; itr != end; ++itr)
        *itr = fn (idx++);
}

} // namespace libcpp

#endif