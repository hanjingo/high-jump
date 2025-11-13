#include <gtest/gtest.h>
#include <hj/os/asm.h>

TEST(asm, asm_macro_basic)
{
    int a = 1, b = 2, c = 0;
#if defined(_MSC_VER) && defined(_M_IX86)
    ASM_BEGIN
    mov eax, a add eax, b mov c, eax ASM_END ASSERT_EQ(c, 3);
// Only use the x86 inline-asm variant when compiling for x86 (32/64-bit)
// Avoid trying to compile x86 registers (like eax) on non-x86 platforms
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    int result = 0;
    ASM_VOLATILE(
        "movl %[a], %%eax; addl %[b], %%eax; movl %%eax, %[c]" : [c] "=r"(
            result) : [a] "r"(a),
        [b] "r"(b) : "eax");
    ASSERT_EQ(result, 3);
#else
    SUCCEED();
#endif
}

TEST(asm, asm_macro_empty)
{
    ASM("");
    ASM_V("");
    SUCCEED();
}

TEST(asm, platform_basic)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    // MSVC x86
    int a = 1, b = 2, c = 0;
    ASM_BEGIN
    mov eax, a add eax, b mov c, eax ASM_END ASSERT_EQ(c, 3);
#elif defined(__GNUC__) && defined(__x86_64__)
    // GCC x86_64
    int a = 1, b = 2, c = 0;
    ASM_VOLATILE(
        "movl %[a], %%eax; addl %[b], %%eax; movl %%eax, %[c]" : [c] "=r"(
            c) : [a] "r"(a),
        [b] "r"(b) : "eax");
    ASSERT_EQ(c, 3);
#elif defined(__aarch64__)
    // ARM64
    int a = 1, b = 2, c = 0;
    ASM_VOLATILE("add %[c], %[a], %[b]" : [c] "=r"(c) : [a] "r"(a), [b] "r"(b));
    ASSERT_EQ(c, 3);
#else
    SUCCEED();
#endif
}

TEST(asm, asm_empty)
{
    ASM("");
    ASM_V("");
    SUCCEED();
}

TEST(asm, asm_invalid)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    // ASM_BEGIN invalid_instruction ASM_END;
    SUCCEED();
#else
    SUCCEED();
#endif
}