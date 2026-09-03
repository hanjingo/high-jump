#ifndef COMPAT_HPP
#define COMPAT_HPP

// -------------------------------- for c++ only --------------------------------
#ifdef __cplusplus

// -------- detect compiler C++ version ------------
#if defined(_MSC_VER) && defined(_MSVC_LANG)
#define HJ_DETAIL_ACTUAL_CPP_VERSION _MSVC_LANG
#else
#define HJ_DETAIL_ACTUAL_CPP_VERSION __cplusplus
#endif

#ifdef HJ_OVERRIDE_CPP_VERSION
#define HJ_DETAIL_CPP_VERSION HJ_OVERRIDE_CPP_VERSION
#else
#define HJ_DETAIL_CPP_VERSION HJ_DETAIL_ACTUAL_CPP_VERSION
#endif

#ifdef HJ_CPP_VERSION
#error                                                                         \
    "HJ_CPP_VERSION is a read-only macro exported by hj. Do not define it manually; use HJ_OVERRIDE_CPP_VERSION instead."
#else
#define HJ_CPP_VERSION HJ_DETAIL_CPP_VERSION
#endif

// -------- avoid redefining std::min/std::max in windows ------------
#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#endif // __cplusplus

// ------------------------ Macros (C/C++ Shared) ------------------------
// Deprecated attribute
#if defined(__cplusplus) && defined(__has_cpp_attribute)                       \
    && __has_cpp_attribute(deprecated) >= 201309L
#define HJ_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(_MSC_VER)
#define HJ_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
#define HJ_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define HJ_DEPRECATED(msg)
#endif

// Inline control
#if defined(_MSC_VER)
#define HJ_FORCE_INLINE __forceinline
#define HJ_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define HJ_FORCE_INLINE __attribute__((always_inline)) inline
#define HJ_NO_INLINE __attribute__((noinline))
#else
#define HJ_FORCE_INLINE inline
#define HJ_NO_INLINE
#endif

// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
#define HJ_LIKELY(x) __builtin_expect(!!(x), 1)
#define HJ_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define HJ_HOT __attribute__((hot))
#define HJ_COLD __attribute__((cold))
#else
#define HJ_LIKELY(x) (!!(x))
#define HJ_UNLIKELY(x) (!!(x))
#define HJ_HOT
#define HJ_COLD
#endif

#endif // COMPAT_HPP