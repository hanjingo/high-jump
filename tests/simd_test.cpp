#include <gtest/gtest.h>
#include <vector>
#include <cmath>

#define HJ_SIMD_IMPL
#include <hj/hardware/simd.h>

std::vector<float> generate_data(size_t n, float start_val)
{
    std::vector<float> data(n);
    for(size_t i = 0; i < n; ++i)
        data[i] = start_val + static_cast<float>(i);
    return data;
}

TEST(simd, add_f32_various_sizes)
{
    const size_t sizes[] = {0, 1, 3, 4, 7, 8, 15, 16};
    for(size_t n : sizes)
    {
        auto               a = generate_data(n, 1.0f);
        auto               b = generate_data(n, 2.0f);
        std::vector<float> out(n, 0.0f);
        hj_simd_add_f32(a.data(), b.data(), out.data(), n);
        for(size_t i = 0; i < n; ++i)
        {
            EXPECT_FLOAT_EQ(out[i], a[i] + b[i])
                << "Failed at size " << n << ", index " << i;
        }
    }
}

TEST(simd, mul_f32_various_sizes)
{
    const size_t sizes[] = {0, 1, 4, 8, 13};
    for(size_t n : sizes)
    {
        auto               a = generate_data(n, 0.5f);
        auto               b = generate_data(n, 1.5f);
        std::vector<float> out(n, 0.0f);
        hj_simd_mul_f32(a.data(), b.data(), out.data(), n);
        for(size_t i = 0; i < n; ++i)
        {
            EXPECT_FLOAT_EQ(out[i], a[i] * b[i])
                << "Failed at size " << n << ", index " << i;
        }
    }
}

TEST(simd, dot_f32_various_sizes)
{
    const size_t sizes[] = {0, 1, 4, 8, 9, 16, 17};
    for(size_t n : sizes)
    {
        auto  a        = generate_data(n, 1.0f);
        auto  b        = generate_data(n, 0.5f);
        float expected = 0.0f;
        for(size_t i = 0; i < n; ++i)
            expected += a[i] * b[i];

        float result = hj_simd_dot_f32(a.data(), b.data(), n);
        EXPECT_NEAR(result, expected, 1e-4f)
            << "Failed dot product at size " << n;
    }
}

TEST(simd, edge_cases)
{
    float out[1] = {999.0f};
    hj_simd_add_f32(nullptr, nullptr, out, 0);
    EXPECT_FLOAT_EQ(out[0], 999.0f);

    float a[] = {-1.0f, 1e6f};
    float b[] = {2.0f, -1e6f};
    float out_mul[2];
    hj_simd_mul_f32(a, b, out_mul, 2);
    EXPECT_FLOAT_EQ(out_mul[0], -2.0f);
    EXPECT_FLOAT_EQ(out_mul[1], -1e12f);
}
