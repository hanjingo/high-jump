#ifndef VARIANT_HPP
#define VARIANT_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <variant>

namespace hj
{
template <typename... T>
using variant = std::variant<T...>;

template <class T, class... Types>
constexpr T &get(std::variant<Types...> &v)
{
    return std::get<T>(v);
};

template <class T, class... Types>
constexpr T &&get(std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
};

template <class T, class... Types>
constexpr const T &get(const std::variant<Types...> &v)
{
    return std::get<T>(v);
};

template <class T, class... Types>
constexpr const T &&get(const std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
};

}

#else
#include <boost/variant.hpp>

namespace hj
{
template <typename... T>
using variant = boost::variant<T...>;

template <class T, class... Types>
constexpr T &get(boost::variant<Types...> &v)
{
    return boost::get<T>(v);
};

template <class T, class... Types>
constexpr T &&get(boost::variant<Types...> &&v)
{
    return boost::get<T>(boost::move(v));
};

template <class T, class... Types>
constexpr const T &get(const boost::variant<Types...> &v)
{
    return boost::get<T>(v);
};

template <class T, class... Types>
constexpr const T &&get(const boost::variant<Types...> &&v)
{
    return boost::get<T>(std::move(v));
};

}
#endif

#endif