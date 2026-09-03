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

#ifndef HJ_OS_ASM_H
#define HJ_OS_ASM_H

/* Compiler Level */
#define HJ_COMPILER_GCC 0
#define HJ_COMPILER_CLANG 0
#define HJ_COMPILER_MSVC 0

/* Architecture Level */
#define HJ_ARCH_X86 0
#define HJ_ARCH_X86_32 0
#define HJ_ARCH_X86_64 0

#define HJ_ARCH_ARM 0
#define HJ_ARCH_ARM32 0
#define HJ_ARCH_ARM64 0

#define HJ_ARCH_RISCV 0
#define HJ_ARCH_RISCV32 0
#define HJ_ARCH_RISCV64 0

/* Feature Level */
#define HJ_HAS_EXTENDED_INLINE_ASM 0
#define HJ_HAS_MSVC_INLINE_ASM 0 // Legacy MSVC x86-32 bit only
#define HJ_HAS_INLINE_ASM 0
#define HJ_HAS_INTRINSICS 1

/* -------------------------------------------------------------------------- */
/* Compiler Detection                                                         */
/* -------------------------------------------------------------------------- */

#if defined(__clang__)
#undef HJ_COMPILER_CLANG
#define HJ_COMPILER_CLANG 1
#elif defined(__GNUC__)
#undef HJ_COMPILER_GCC
#define HJ_COMPILER_GCC 1
#endif

#if defined(_MSC_VER)
#undef HJ_COMPILER_MSVC
#define HJ_COMPILER_MSVC 1
#endif

/* -------------------------------------------------------------------------- */
/* Architecture Detection                                                     */
/* -------------------------------------------------------------------------- */

#if defined(__i386__) || defined(_M_IX86)
#undef HJ_ARCH_X86_32
#define HJ_ARCH_X86_32 1
#undef HJ_ARCH_X86
#define HJ_ARCH_X86 1
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#undef HJ_ARCH_X86_64
#define HJ_ARCH_X86_64 1
#undef HJ_ARCH_X86
#define HJ_ARCH_X86 1
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#undef HJ_ARCH_ARM64
#define HJ_ARCH_ARM64 1
#undef HJ_ARCH_ARM
#define HJ_ARCH_ARM 1
#elif defined(__arm__) || defined(_M_ARM)
#undef HJ_ARCH_ARM32
#define HJ_ARCH_ARM32 1
#undef HJ_ARCH_ARM
#define HJ_ARCH_ARM 1
#endif

#if defined(__riscv)
#undef HJ_ARCH_RISCV
#define HJ_ARCH_RISCV 1
#if __riscv_xlen == 64
#undef HJ_ARCH_RISCV64
#define HJ_ARCH_RISCV64 1
#elif __riscv_xlen == 32
#undef HJ_ARCH_RISCV32
#define HJ_ARCH_RISCV32 1
#endif
#endif

/* -------------------------------------------------------------------------- */
/* Assembly Feature Detection                                                 */
/* -------------------------------------------------------------------------- */

/* GCC 风格 Extended Inline ASM */
#if (HJ_COMPILER_GCC || HJ_COMPILER_CLANG) && !defined(__ASSEMBLER__)
#undef HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_HAS_EXTENDED_INLINE_ASM 1
#endif

/* MSVC 传统 Inline ASM (仅 Legacy 32位 MSVC x86 支持) */
#if HJ_COMPILER_MSVC && HJ_ARCH_X86_32
#undef HJ_HAS_MSVC_INLINE_ASM
#define HJ_HAS_MSVC_INLINE_ASM 1
#endif

#if HJ_HAS_EXTENDED_INLINE_ASM || HJ_HAS_MSVC_INLINE_ASM
#undef HJ_HAS_INLINE_ASM
#define HJ_HAS_INLINE_ASM 1
#endif

/* -------------------------------------------------------------------------- */
/* Assembly Modifiers (Only defined if the current platform supports assembly)   */
/* -------------------------------------------------------------------------- */

#if HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_ASM __asm__
#define HJ_ASM_VOLATILE __asm__ __volatile__
#elif HJ_HAS_MSVC_INLINE_ASM
#define HJ_ASM __asm
#define HJ_ASM_VOLATILE __asm
#endif
/* 注意：不支持内联汇编的平台故意不定义 HJ_ASM / HJ_ASM_VOLATILE，
   防止代码在未受保护的情况下使用裸汇编宏产生错误的静默展开。 */

/* -------------------------------------------------------------------------- */
/* Intrinsics Headers                                                          */
/* -------------------------------------------------------------------------- */

#if HJ_COMPILER_MSVC || defined(__clang_cli__)
#include <intrin.h>
#elif HJ_COMPILER_GCC || HJ_COMPILER_CLANG
#if HJ_ARCH_X86
#include <x86intrin.h>
#elif HJ_ARCH_ARM
#include <arm_acle.h>
#endif
#endif

/* -------------------------------------------------------------------------- */
/* Cross-Platform CPU Primitives & Barriers                                     */
/* -------------------------------------------------------------------------- */
#if HJ_COMPILER_MSVC
#define HJ_COMPILER_BARRIER() _ReadWriteBarrier()
#elif HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#else
#define HJ_COMPILER_BARRIER() ((void) 0)
#endif

#if HJ_ARCH_X86
#if HJ_COMPILER_MSVC
#define HJ_ASM_PAUSE() _mm_pause()
#elif HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_ASM_PAUSE() __asm__ __volatile__("pause")
#else
#define HJ_ASM_PAUSE() ((void) 0)
#endif
#elif HJ_ARCH_ARM
#if HJ_COMPILER_MSVC
#define HJ_ASM_PAUSE() __yield()
#elif HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_ASM_PAUSE() __asm__ __volatile__("yield")
#else
#define HJ_ASM_PAUSE() ((void) 0)
#endif
#elif HJ_ARCH_RISCV
#if HJ_HAS_EXTENDED_INLINE_ASM && defined(__riscv_zihintpause)
#define HJ_ASM_PAUSE() __asm__ __volatile__("pause")
#else
#define HJ_ASM_PAUSE() ((void) 0)
#endif
#else
#define HJ_ASM_PAUSE() ((void) 0)
#endif

#if HJ_COMPILER_MSVC
#define HJ_ASM_NOP() __nop()
#elif HJ_HAS_EXTENDED_INLINE_ASM
#define HJ_ASM_NOP() __asm__ __volatile__("nop")
#else
#define HJ_ASM_NOP() ((void) 0)
#endif

#endif // HJ_OS_ASM_H