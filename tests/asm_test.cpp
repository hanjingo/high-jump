#include <gtest/gtest.h>
#include <hj/os/asm.h>

TEST(asm, feature_detection)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    EXPECT_TRUE(HJ_HAS_MSVC_INLINE_ASM);
    EXPECT_TRUE(HJ_HAS_INLINE_ASM);
#elif defined(__GNUC__) || defined(__clang__)
    EXPECT_TRUE(HJ_HAS_EXTENDED_INLINE_ASM);
    EXPECT_TRUE(HJ_HAS_INLINE_ASM);
#else
    EXPECT_FALSE(HJ_HAS_INLINE_ASM);
#endif

#if defined(_M_X64) || defined(__x86_64__)
    EXPECT_TRUE(HJ_ARCH_X86_64);
    EXPECT_TRUE(HJ_ARCH_X86);
#elif defined(_M_IX86) || defined(__i386__)
    EXPECT_TRUE(HJ_ARCH_X86_32);
    EXPECT_TRUE(HJ_ARCH_X86);
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
    EXPECT_TRUE(HJ_ARCH_ARM64);
    EXPECT_TRUE(HJ_ARCH_ARM);
#elif defined(__arm__) || defined(_M_ARM)
    EXPECT_TRUE(HJ_ARCH_ARM32);
    EXPECT_TRUE(HJ_ARCH_ARM);
#endif

#if defined(__riscv)
    EXPECT_TRUE(HJ_ARCH_RISCV);
#if __riscv_xlen == 64
    EXPECT_TRUE(HJ_ARCH_RISCV64);
#elif __riscv_xlen == 32
    EXPECT_TRUE(HJ_ARCH_RISCV32);
#endif
#endif
}

TEST(asm, compiler_barrier_memory_flush)
{
    int           memory_var = 100;
    volatile int *p          = &memory_var;

    *p = 200;

    HJ_COMPILER_BARRIER();

    EXPECT_EQ(*p, 200);

    *p = 300;
    HJ_COMPILER_BARRIER();

    EXPECT_EQ(memory_var, 300);
}

TEST(asm, cpu_pause)
{
    HJ_ASM_PAUSE();
    SUCCEED();
}

TEST(asm, platform_inline_asm_execution)
{
    int a = 1, b = 2, c = 0;

#if HJ_HAS_EXTENDED_INLINE_ASM

#if HJ_ARCH_X86
    HJ_ASM_VOLATILE("addl %[b], %[a]" : [a] "+r"(a) : [b] "r"(b));
    c = a;
    EXPECT_EQ(c, 3);

#elif HJ_ARCH_ARM64
    HJ_ASM_VOLATILE("add %[c], %[a], %[b]" : [c] "=r"(c) : [a] "r"(a),
                    [b] "r"(b));
    EXPECT_EQ(c, 3);

#elif HJ_ARCH_ARM32
    HJ_ASM_VOLATILE("add %[c], %[a], %[b]" : [c] "=r"(c) : [a] "r"(a),
                    [b] "r"(b));
    EXPECT_EQ(c, 3);

#elif HJ_ARCH_RISCV
    HJ_ASM_VOLATILE("add %[c], %[a], %[b]" : [c] "=r"(c) : [a] "r"(a),
                    [b] "r"(b));
    EXPECT_EQ(c, 3);

#else
    c = a + b;
    EXPECT_EQ(c, 3);
#endif

#elif HJ_HAS_MSVC_INLINE_ASM
    HJ_ASM{mov eax, a add eax, b mov c, eax} EXPECT_EQ(c, 3);
#else
    c = a + b;
    EXPECT_EQ(c, 3);
#endif
}