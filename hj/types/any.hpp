#ifndef ANY_HPP
#define ANY_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <any>

namespace hj
{

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

}

#else
#include <boost/any.hpp>

namespace hj
{

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

}
#endif

#endif