#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <ciso646>

#define HJ_HAS_VARIANT 0

#if defined(__has_include)
#if __has_include(<variant>) && ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#undef HJ_HAS_VARIANT
#define HJ_HAS_VARIANT 1
#endif
#endif

#if HJ_HAS_VARIANT
#include <variant>

namespace hj
{

template <typename... T>
using variant = std::variant<T...>;

template <class T, class... Types>
constexpr T &get(std::variant<Types...> &v)
{
    return std::get<T>(v);
}

template <class T, class... Types>
constexpr T &&get(std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
}

template <class T, class... Types>
constexpr const T &get(const std::variant<Types...> &v)
{
    return std::get<T>(v);
}

template <class T, class... Types>
constexpr const T &&get(const std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
}

template <size_t I, class... Types>
constexpr auto &get(std::variant<Types...> &v)
{
    return std::get<I>(v);
}

template <size_t I, class... Types>
constexpr auto &&get(std::variant<Types...> &&v)
{
    return std::get<I>(std::move(v));
}

template <size_t I, class... Types>
constexpr const auto &get(const std::variant<Types...> &v)
{
    return std::get<I>(v);
}

template <size_t I, class... Types>
constexpr const auto &&get(const std::variant<Types...> &&v)
{
    return std::get<I>(std::move(v));
}

template <class T, class... Types>
constexpr T *get_if(std::variant<Types...> *v) noexcept
{
    return std::get_if<T>(v);
}

template <class T, class... Types>
constexpr const T *get_if(const std::variant<Types...> *v) noexcept
{
    return std::get_if<T>(v);
}

template <size_t I, class... Types>
constexpr auto *get_if(std::variant<Types...> *v) noexcept
{
    return std::get_if<I>(v);
}

template <size_t I, class... Types>
constexpr const auto *get_if(const std::variant<Types...> *v) noexcept
{
    return std::get_if<I>(v);
}

template <class T, class... Types>
constexpr bool holds_alternative(const std::variant<Types...> &v) noexcept
{
    return std::holds_alternative<T>(v);
}

template <class Visitor, class... Variants>
constexpr decltype(auto) visit(Visitor &&vis, Variants &&...vars)
{
    return std::visit(std::forward<Visitor>(vis),
                      std::forward<Variants>(vars)...);
}

template <typename T>
struct variant_size : std::variant_size<T>
{
};

template <size_t I, typename T>
struct variant_alternative : std::variant_alternative<I, T>
{
};

template <typename T>
inline constexpr size_t variant_size_v = std::variant_size_v<T>;

template <size_t I, typename T>
using variant_alternative_t = std::variant_alternative_t<I, T>;

} // namespace hj

#else

#define HJ_HAS_BOOST_VARIANT 0
#if defined(__has_include)
#if __has_include(<boost/variant.hpp>)
#undef HJ_HAS_BOOST_VARIANT
#define HJ_HAS_BOOST_VARIANT 1
#endif
#elif defined(BOOST_HAS_VARIANT_HPP)
#define HJ_HAS_BOOST_VARIANT 1
#endif

#if HJ_HAS_BOOST_VARIANT
#include <boost/variant.hpp>
#include <boost/move/utility.hpp>

namespace hj
{

template <typename... T>
using variant = boost::variant<T...>;

template <class T, class... Types>
T &get(boost::variant<Types...> &v)
{
    return boost::get<T>(v);
}

template <class T, class... Types>
T &&get(boost::variant<Types...> &&v)
{
    return boost::get<T>(boost::move(v));
}

template <class T, class... Types>
const T &get(const boost::variant<Types...> &v)
{
    return boost::get<T>(v);
}

template <class T, class... Types>
const T &&get(const boost::variant<Types...> &&v)
{
    return boost::get<T>(boost::move(v));
}

template <size_t I, class... Types>
auto &get(boost::variant<Types...> &v)
{
    return *boost::get<I>(&v);
}

template <size_t I, class... Types>
auto &&get(boost::variant<Types...> &&v)
{
    return std::move(*boost::get<I>(&v));
}

template <size_t I, class... Types>
const auto &get(const boost::variant<Types...> &v)
{
    return *boost::get<I>(&v);
}

template <size_t I, class... Types>
const auto &&get(const boost::variant<Types...> &&v)
{
    return std::move(*boost::get<I>(&v));
}

template <class T, class... Types>
T *get_if(boost::variant<Types...> *v) noexcept
{
    return boost::get<T>(v);
}

template <class T, class... Types>
const T *get_if(const boost::variant<Types...> *v) noexcept
{
    return boost::get<T>(v);
}

template <size_t I, class... Types>
auto *get_if(boost::variant<Types...> *v) noexcept
{
    return boost::get<I>(v);
}

template <size_t I, class... Types>
const auto *get_if(const boost::variant<Types...> *v) noexcept
{
    return boost::get<I>(v);
}

template <class T, class... Types>
bool holds_alternative(const boost::variant<Types...> &v) noexcept
{
    return boost::get<T>(&v) != nullptr;
}

template <class Visitor, class... Variants>
decltype(auto) visit(Visitor &&vis, Variants &&...vars)
{
    return boost::apply_visitor(std::forward<Visitor>(vis),
                                std::forward<Variants>(vars)...);
}

template <typename T>
struct variant_size : boost::variant_size<T>
{
};

template <size_t I, typename T>
struct variant_alternative : boost::variant_alternative<I, T>
{
};

template <typename T>
inline constexpr size_t variant_size_v = boost::variant_size<T>::value;

template <size_t I, typename T>
using variant_alternative_t = typename boost::variant_alternative<I, T>::type;

} // namespace hj

#else
#error                                                                         \
    "No suitable variant implementation found (need C++17 std::variant or boost::variant)"
#endif

#endif

#endif // VARIANT_HPP