/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SIMD_H
#define SIMD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_SIMD_API
#if defined(HJ_SIMD_STATIC)
#define HJ_SIMD_API static inline
#else
#define HJ_SIMD_API extern
#endif
#endif

// ------------------------ ROM API Declarations ------------------------
HJ_SIMD_API void hj_simd_add_f32(const float *__restrict a,
                                 const float *__restrict b,
                                 float *__restrict out,
                                 size_t n);
HJ_SIMD_API void hj_simd_mul_f32(const float *__restrict a,
                                 const float *__restrict b,
                                 float *__restrict out,
                                 size_t n);
HJ_SIMD_API float
hj_simd_dot_f32(const float *__restrict a, const float *__restrict b, size_t n);

#ifdef __cplusplus
}
#endif

#endif // SIMD_H

// --------------------- Implementation -------------------------
#if (defined(HJ_SIMD_IMPL) || defined(HJ_SIMD_STATIC))                         \
    && !defined(HJ_SIMD_IMPL_DONE)
#define HJ_SIMD_IMPL_DONE

#if defined(__AVX2__)
#include <immintrin.h>
#define SIMD_AVX2
#endif
#if defined(__FMA__)
#define SIMD_FMA
#endif

#if defined(__SSE__) || defined(_M_IX86_FP)
#include <xmmintrin.h>
#define SIMD_SSE
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define SIMD_NEON
#endif

#ifdef __cplusplus
extern "C" {
#endif

HJ_SIMD_API void hj_simd_add_f32(const float *__restrict a,
                                 const float *__restrict b,
                                 float *__restrict out,
                                 size_t n)
{
    size_t i = 0;
#if defined(SIMD_AVX2)
    for(; i + 8 <= n; i += 8)
    {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_add_ps(va, vb));
    }
#elif defined(SIMD_SSE)
    for(; i + 4 <= n; i += 4)
    {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        _mm_storeu_ps(out + i, _mm_add_ps(va, vb));
    }
#elif defined(SIMD_NEON)
    for(; i + 4 <= n; i += 4)
    {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);
        vst1q_f32(out + i, vaddq_f32(va, vb));
    }
#endif
    for(; i < n; ++i)
        out[i] = a[i] + b[i];
}

HJ_SIMD_API void hj_simd_mul_f32(const float *__restrict a,
                                 const float *__restrict b,
                                 float *__restrict out,
                                 size_t n)
{
    size_t i = 0;
#if defined(SIMD_AVX2)
    for(; i + 8 <= n; i += 8)
    {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_mul_ps(va, vb));
    }
#elif defined(SIMD_SSE)
    for(; i + 4 <= n; i += 4)
    {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        _mm_storeu_ps(out + i, _mm_mul_ps(va, vb));
    }
#elif defined(SIMD_NEON)
    for(; i + 4 <= n; i += 4)
    {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);
        vst1q_f32(out + i, vmulq_f32(va, vb));
    }
#endif
    for(; i < n; ++i)
        out[i] = a[i] * b[i];
}

HJ_SIMD_API float
hj_simd_dot_f32(const float *__restrict a, const float *__restrict b, size_t n)
{
    float  result = 0.0f;
    size_t i      = 0;
#if defined(SIMD_AVX2)
    __m256 vsum = _mm256_setzero_ps();
    for(; i + 8 <= n; i += 8)
    {
#if defined(SIMD_FMA)
        vsum = _mm256_fmadd_ps(_mm256_loadu_ps(a + i),
                               _mm256_loadu_ps(b + i),
                               vsum);
#else
        vsum = _mm256_add_ps(
            vsum,
            _mm256_mul_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i)));
#endif
    }
    float tmp[8];
    _mm256_storeu_ps(tmp, vsum);
    for(int j = 0; j < 8; ++j)
        result += tmp[j];
#elif defined(SIMD_SSE)
    __m128 vsum = _mm_setzero_ps();
    for(; i + 4 <= n; i += 4)
    {
        vsum = _mm_add_ps(vsum,
                          _mm_mul_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i)));
    }
    vsum   = _mm_add_ps(vsum, _mm_movehl_ps(vsum, vsum));
    vsum   = _mm_add_ss(vsum, _mm_shuffle_ps(vsum, vsum, 0x55));
    result = _mm_cvtss_f32(vsum);
#elif defined(SIMD_NEON)
    float32x4_t vsum = vdupq_n_f32(0.0f);
    for(; i + 4 <= n; i += 4)
    {
        vsum = vfmaq_f32(vsum, vld1q_f32(a + i), vld1q_f32(b + i));
    }
    result = vgetq_lane_f32(vsum, 0) + vgetq_lane_f32(vsum, 1)
             + vgetq_lane_f32(vsum, 2) + vgetq_lane_f32(vsum, 3);
#endif
    for(; i < n; ++i)
        result += a[i] * b[i];
    return result;
}

#ifdef __cplusplus
}
#endif
#endif // HJ_SIMD_IMPL_DONE
