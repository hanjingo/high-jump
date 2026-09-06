#include <cstdint>
#include <cstring>
#include <limits>
#include <gtest/gtest.h>
#include <hj/encoding/endian.hpp>

namespace
{

bool is_system_big_endian() noexcept
{
    uint16_t val = 0x0100;
    uint8_t  bytes[sizeof(uint16_t)];
    std::memcpy(bytes, &val, sizeof(uint16_t));
    return bytes[0] == 0x01;
}

} // namespace

TEST(endian, constexpr_compile_time_evaluation)
{
    constexpr uint16_t be16 = hj::to_big_endian(static_cast<uint16_t>(0x1234U));
    constexpr uint32_t be32 =
        hj::to_big_endian(static_cast<uint32_t>(0x01020304U));
    constexpr uint64_t be64 =
        hj::to_big_endian(static_cast<uint64_t>(0x0102030405060708ULL));

    static_assert(hj::from_big_endian(be16) == 0x1234U,
                  "Compile-time roundtrip failed for 16-bit");
    static_assert(hj::from_big_endian(be32) == 0x01020304U,
                  "Compile-time roundtrip failed for 32-bit");
    static_assert(hj::from_big_endian(be64) == 0x0102030405060708ULL,
                  "Compile-time roundtrip failed for 64-bit");
}

TEST(endian, type_constraints)
{
    static_assert(hj::detail::is_valid_endian_type_v<int8_t>);
    static_assert(hj::detail::is_valid_endian_type_v<uint8_t>);
    static_assert(hj::detail::is_valid_endian_type_v<int16_t>);
    static_assert(hj::detail::is_valid_endian_type_v<uint16_t>);
    static_assert(hj::detail::is_valid_endian_type_v<int32_t>);
    static_assert(hj::detail::is_valid_endian_type_v<uint32_t>);
    static_assert(hj::detail::is_valid_endian_type_v<int64_t>);
    static_assert(hj::detail::is_valid_endian_type_v<uint64_t>);

    static_assert(!hj::detail::is_valid_endian_type_v<bool>);
    static_assert(!hj::detail::is_valid_endian_type_v<char>);
    static_assert(!hj::detail::is_valid_endian_type_v<wchar_t>);
    static_assert(!hj::detail::is_valid_endian_type_v<char16_t>);
    static_assert(!hj::detail::is_valid_endian_type_v<char32_t>);
}

TEST(endian, known_byte_patterns)
{
    const bool is_be = is_system_big_endian();

    // 16-bit
    uint16_t v16 = 0x0102U;
    EXPECT_EQ(hj::to_big_endian(v16), is_be ? 0x0102U : 0x0201U);
    EXPECT_EQ(hj::to_little_endian(v16), is_be ? 0x0201U : 0x0102U);

    // 32-bit
    uint32_t v32 = 0x01020304U;
    EXPECT_EQ(hj::to_big_endian(v32), is_be ? 0x01020304U : 0x04030201U);
    EXPECT_EQ(hj::to_little_endian(v32), is_be ? 0x04030201U : 0x01020304U);

    // 64-bit
    uint64_t v64 = 0x0102030405060708ULL;
    EXPECT_EQ(hj::to_big_endian(v64),
              is_be ? 0x0102030405060708ULL : 0x0807060504030201ULL);
    EXPECT_EQ(hj::to_little_endian(v64),
              is_be ? 0x0807060504030201ULL : 0x0102030405060708ULL);
}

TEST(endian, boundary_values)
{
    // 0 & 1
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(static_cast<uint16_t>(0))),
              0U);
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(static_cast<uint16_t>(1))),
              1U);

    // INT16 Min / Max
    int16_t i16_min = std::numeric_limits<int16_t>::min();
    int16_t i16_max = std::numeric_limits<int16_t>::max();
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i16_min)), i16_min);
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i16_max)), i16_max);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i16_min)), i16_min);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i16_max)), i16_max);

    // INT32 Min / Max
    int32_t i32_min = std::numeric_limits<int32_t>::min();
    int32_t i32_max = std::numeric_limits<int32_t>::max();
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i32_min)), i32_min);
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i32_max)), i32_max);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i32_min)), i32_min);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i32_max)), i32_max);

    // INT64 Min / Max
    int64_t i64_min = std::numeric_limits<int64_t>::min();
    int64_t i64_max = std::numeric_limits<int64_t>::max();
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i64_min)), i64_min);
    EXPECT_EQ(hj::from_big_endian(hj::to_big_endian(i64_max)), i64_max);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i64_min)), i64_min);
    EXPECT_EQ(hj::from_little_endian(hj::to_little_endian(i64_max)), i64_max);
}