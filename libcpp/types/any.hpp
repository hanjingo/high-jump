#ifndef ANY_HPP
#define ANY_HPP

#if (__cplusplus >= 201703L)
#include <any>

namespace libcpp {

using any = std::any;

template <class T>
constexpr T* any_cast(any* operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
constexpr const T* any_cast(const any* operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
constexpr T any_cast(any& operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
constexpr T any_cast(const any& operand)
{
    return std::any_cast<T>(operand);
}

}  // namespace libcpp
#endif

#if (__cplusplus < 201703L)
#include <boost/any.hpp>

namespace libcpp {

using any = boost::any;

template <class T>
constexpr T* any_cast(any* operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
constexpr const T* any_cast(const any* operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
constexpr T any_cast(any& operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
constexpr T any_cast(const any& operand)
{
    return boost::any_cast<T>(operand);
}

}  // namespace libcpp
#endif

#endif