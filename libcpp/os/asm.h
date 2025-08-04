#ifndef ASM_H
#define ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_MSC_VER)
#define ASM_BEGIN __asm {
#define ASM_END }
#define ASM_INLINE __asm
#define ASM_VOLATILE

#define ASM(code) __asm { code }
#define ASM_V(code) __asm { code }

#elif defined(__GNUC__) || defined(__clang__)
#define ASM_BEGIN   asm (
#define ASM_END     );
#define ASM_INLINE asm
#define ASM_VOLATILE asm volatile

#define ASM(code) asm(code)
#define ASM_V(code) asm volatile(code)

#else
#define ASM_BEGIN
#define ASM_END
#define ASM_INLINE
#define ASM_VOLATILE
#define ASM(code)
#define ASM_V(code)
#endif

#ifdef __cplusplus
}
#endif

#endif  // ASM_H