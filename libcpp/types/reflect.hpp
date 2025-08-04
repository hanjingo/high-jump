#ifndef REFLECT_HPP
#define REFLECT_HPP

#include <string>
#include <type_traits>
#include <typeinfo>

#include <boost/core/demangle.hpp>

namespace libcpp
{
class reflect
{
  public:
    template <typename T> static std::string type_name (const T &t)
    {
        return boost::core::demangle (typeid (t).name ());
    }

    template <typename T> static bool is_pod (T t)
    {
        return std::is_pod<T>::value;
    }

    // template<typename T>
    // static T copy()
    // {

    // }

    // template<typename T>
    // static T clone()
    // {

    // }

    // template <typename T>
    // unsigned char* serialize()
    // {

    // }

    // template <typename T>
    // T unserialize(const unsigned char* buf)
    // {

    // }

    template <typename T1, typename T2> static bool is_same_type (T1 t1, T2 t2)
    {
        return std::is_same<T1, T2>::value;
    }

    template <typename T1, typename T2, typename T3>
    static bool is_same_type (T1 t1, T2 t2, T3 t3)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value;
    }

    template <typename T1, typename T2, typename T3, typename T4>
    static bool is_same_type (T1 t1, T2 t2, T3 t3, T4 t4)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value
               && std::is_same<T1, T4>::value;
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    static bool is_same_type (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value
               && std::is_same<T1, T4>::value && std::is_same<T1, T5>::value;
    }

    template <typename T1,
              typename T2,
              typename T3,
              typename T4,
              typename T5,
              typename T6>
    static bool is_same_type (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value
               && std::is_same<T1, T4>::value && std::is_same<T1, T5>::value
               && std::is_same<T1, T6>::value;
    }

    template <typename T1,
              typename T2,
              typename T3,
              typename T4,
              typename T5,
              typename T6,
              typename T7>
    static bool is_same_type (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value
               && std::is_same<T1, T4>::value && std::is_same<T1, T5>::value
               && std::is_same<T1, T6>::value && std::is_same<T1, T7>::value;
    }
};

} // namespace libcpp

#endif
