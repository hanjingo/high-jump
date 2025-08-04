#include <cmath>
#include <vector>

#include <gtest/gtest.h>
#include <libcpp/hardware/simd.h>

TEST(SIMDTest, AddF32)
{
    float a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    float b[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };
    float out[8] = { 0 };
    simd_add_f32(a, b, out, 8);
    for (int i = 0; i < 8; ++i)
        EXPECT_FLOAT_EQ(out[i], a[i] + b[i]);
}

TEST(SIMDTest, MulF32)
{
    float a[4] = { 1.5f, 2.0f, -3.0f, 0.5f };
    float b[4] = { 2.0f, -1.0f, 4.0f, 8.0f };
    float out[4] = { 0 };
    simd_mul_f32(a, b, out, 4);
    for (int i = 0; i < 4; ++i)
        EXPECT_FLOAT_EQ(out[i], a[i] * b[i]);
}

TEST(SIMDTest, DotF32)
{
    float a[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float b[4] = { 5.0f, 6.0f, 7.0f, 8.0f };
    float expected = 1 * 5 + 2 * 6 + 3 * 7 + 4 * 8;
    float result = simd_dot_f32(a, b, 4);
    EXPECT_NEAR(result, expected, 1e-5f);
}

TEST(SIMDTest, DotF32_NonMultipleOf4)
{
    float a[6] = { 1, 2, 3, 4, 5, 6 };
    float b[6] = { 6, 5, 4, 3, 2, 1 };
    float expected = 1 * 6 + 2 * 5 + 3 * 4 + 4 * 3 + 5 * 2 + 6 * 1;
    float result = simd_dot_f32(a, b, 6);
    EXPECT_NEAR(result, expected, 1e-5f);
}
