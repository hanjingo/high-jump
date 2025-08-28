#ifndef REFLECT_HPP
#define REFLECT_HPP

#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <cstring>

#include <boost/core/demangle.hpp>

namespace libcpp
{
class reflect
{
public:
    template<typename T>
    static std::string type_name(const T& t)
    {
        return boost::core::demangle(typeid(t).name());
    }

    template<typename T>
    static bool is_pod(T t)
    {
        return std::is_pod<T>::value;
    }

    template<typename T>
    static T copy(const T& t)
    {
        return t;
    }

    template<typename T>
    static T clone(const T& t)
    {
        return T(t);
    }

    template <typename T>
    static std::vector<unsigned char> serialize(const T& t)
    {
        static_assert(std::is_trivially_copyable<T>::value, "POD only");
        const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&t);
        return std::vector<unsigned char>(ptr, ptr + sizeof(T));
    }

    template <typename T>
    static T unserialize(const unsigned char* buf)
    {
        static_assert(std::is_trivially_copyable<T>::value, "POD only");
        T t;
        std::memcpy(&t, buf, sizeof(T));
        return t;
    }

    template <typename T, typename Member>
    static std::size_t offset_of(Member T::* member)
    {
        return reinterpret_cast<std::size_t>(&(((T*)0)->*member));
    }

    template <typename T, typename Member>
    static std::size_t size_of(Member T::* member)
    {
        return sizeof(((T*)0)->*member);
    }

    // template <typename T, typename Member>
    // static std::size_t align_of(Member T::* member)
    // {
    //     return alignof(((T*)0)->*member);
    // }

    template<typename T1, typename T2>
    static bool is_same_type(const T1& t1, const T2& t2)
    {
        return std::is_same<T1, T2>::value;
    }

    template<typename T1, typename T2, typename T3>
    static bool is_same_type(const T1& t1, const T2& t2, const T3& t3)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value;
    }

    template<typename T1, typename T2, typename T3, typename T4>
    static bool is_same_type(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value &&
               std::is_same<T1, T4>::value;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    static bool is_same_type(const T1& t1, const T2& t2, const T3& t3, const T4& t4, 
                             const T5& t5)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value &&
               std::is_same<T1, T4>::value && std::is_same<T1, T5>::value;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
             typename T6>
    static bool is_same_type(const T1& t1, const T2& t2, const T3& t3, const T4& t4, 
                             const T5& t5, const T6& t6)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value &&
               std::is_same<T1, T4>::value && std::is_same<T1, T5>::value &&
               std::is_same<T1, T6>::value ;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
             typename T6, typename T7>
    static bool is_same_type(const T1& t1, const T2& t2, const T3& t3, const T4& t4, 
                             const T5& t5, const T6& t6, const T7& t7)
    {
        return std::is_same<T1, T2>::value && std::is_same<T1, T3>::value &&
               std::is_same<T1, T4>::value && std::is_same<T1, T5>::value &&
               std::is_same<T1, T6>::value && std::is_same<T1, T7>::value;
    }

};

}

#endif
