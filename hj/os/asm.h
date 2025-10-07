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

#ifndef ASM_H
#define ASM_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_MSC_VER)
#if defined(_M_IX86)
#define ASM_BEGIN __asm {
#define ASM_END }
#define ASM_INLINE __asm
#define ASM_VOLATILE
#define ASM(code) __asm { code }
#define ASM_V(code) __asm { code }
#else
#pragma message("Warning: MSVC x64/ARM does not support inline assembly.")
#define ASM_BEGIN
#define ASM_END
#define ASM_INLINE
#define ASM_VOLATILE
#define ASM(code)
#define ASM_V(code)
#endif

#elif defined(__GNUC__) || defined(__clang__)
#define ASM_BEGIN   asm (
#define ASM_END     );
#define ASM_INLINE asm
#define ASM_VOLATILE asm volatile
#define ASM(code) asm(code)
#define ASM_V(code) asm volatile(code)

#else
#warning "Inline assembly not supported on this platform."
#define ASM_BEGIN
#define ASM_END
#define ASM_INLINE
#define ASM_VOLATILE
#define ASM(code)
#define ASM_V(code)
#endif

#if defined(__cplusplus)
}
#endif

#endif // ASM_H