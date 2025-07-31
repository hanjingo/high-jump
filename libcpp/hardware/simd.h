#ifndef SIMD_H
#define SIMD_H

#include <stdint.h>
#include <stddef.h>

#if defined(__SSE__) || defined(_M_IX86_FP)
    #include <xmmintrin.h>
    #define SIMD_SSE
#endif

#if defined(__AVX__)
    #include <immintrin.h>
    #define SIMD_AVX
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Add two float arrays using SIMD (SSE), fallback to scalar if not available
static inline void simd_add_f32(const float* a, const float* b, float* out, size_t n)
{
#if defined(SIMD_SSE)
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        __m128 vsum = _mm_add_ps(va, vb);
        _mm_storeu_ps(out + i, vsum);
    }
    for (; i < n; ++i)
        out[i] = a[i] + b[i];
#else
    for (size_t i = 0; i < n; ++i)
        out[i] = a[i] + b[i];
#endif
}

// Multiply two float arrays using SIMD (SSE), fallback to scalar if not available
static inline void simd_mul_f32(const float* a, const float* b, float* out, size_t n)
{
#if defined(SIMD_SSE)
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        __m128 vmul = _mm_mul_ps(va, vb);
        _mm_storeu_ps(out + i, vmul);
    }
    for (; i < n; ++i)
        out[i] = a[i] * b[i];
#else
    for (size_t i = 0; i < n; ++i)
        out[i] = a[i] * b[i];
#endif
}

// Compute dot product of two float arrays using SIMD (SSE), fallback to scalar if not available
static inline float simd_dot_f32(const float* a, const float* b, size_t n)
{
    float result = 0.0f;
#if defined(SIMD_SSE)
    __m128 vsum = _mm_setzero_ps();
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        vsum = _mm_add_ps(vsum, _mm_mul_ps(va, vb));
    }
    float tmp[4];
    _mm_storeu_ps(tmp, vsum);
    result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
    for (; i < n; ++i)
        result += a[i] * b[i];
#else
    for (size_t i = 0; i < n; ++i)
        result += a[i] * b[i];
#endif
    return result;
}

#ifdef __cplusplus
}
#endif

#endif // SIMD_H