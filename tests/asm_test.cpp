#include <gtest/gtest.h>
#include <hj/os/asm.h>

TEST(ASMTest, ASM_Macro_Basic)
{
    int a = 1, b = 2, c = 0;
#if defined(_MSC_VER) && defined(_M_IX86)
    ASM_BEGIN
    mov eax, a add eax, b mov c, eax ASM_END ASSERT_EQ(c, 3);
#elif defined(__GNUC__) || defined(__clang__)
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

TEST(ASMTest, ASM_Macro_Empty)
{
    ASM("");
    ASM_V("");
    SUCCEED();
}

TEST(ASMTest, Platform_Basic)
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

TEST(ASMTest, ASM_Empty)
{
    ASM("");
    ASM_V("");
    SUCCEED();
}

TEST(ASMTest, ASM_Invalid)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    // ASM_BEGIN invalid_instruction ASM_END;
    SUCCEED();
#else
    SUCCEED();
#endif
}