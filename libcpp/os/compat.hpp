#ifndef COMPAT_HPP
#define COMPAT_HPP

#include <stdio.h>

// -------------------------------- for c++ --------------------------------
#ifdef __cplusplus
#include <functional>

// c++ std::unary_function compatibility
#ifndef LIBCPP_UNARY_FUNCTION_DEFINED
    #if defined(_MSC_VER)
        #if (_MSC_VER >= 1910)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #endif
    #elif (__cplusplus >= 201703L)
        #if defined(__GLIBCXX__)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #elif defined(_LIBCPP_VERSION)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #endif
    #else
        #define LIBCPP_UNARY_FUNCTION_DEFINED 1
    #endif
#endif

#if !LIBCPP_UNARY_FUNCTION_DEFINED
namespace std {
    template <class Arg, class Result>
    struct unary_function {
        typedef Arg argument_type;
        typedef Result result_type;
    };
}
#undef LIBCPP_UNARY_FUNCTION_DEFINED
#define LIBCPP_UNARY_FUNCTION_DEFINED 1
#endif

#endif



// ------------------------------------ for c/c++ --------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
// deprecated
#if (__cplusplus >= 201402L)
#if defined(_MSC_VER)
#define DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__)
#define DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define DEPRECATED(msg) [[deprecated(msg)]]
#endif
#else
#define DEPRECATED(msg)
#endif


// inline
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
    #define NO_INLINE __attribute__((noinline))
#else
    #define FORCE_INLINE inline
    #define NO_INLINE
#endif


// branch prediction hints (gcc and clang only)
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define HOT         __attribute__((hot))
    #define COLD        __attribute__((cold))
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
    #define HOT
    #define COLD
#endif

#ifdef __cplusplus
}
#endif

#endif // COMPAT_HPP