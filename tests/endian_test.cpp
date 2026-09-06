#include <cstdint>
#include <cstring>
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

TEST(endian, is_big_endian)
{
    ASSERT_EQ(hj::is_big_endian(), is_system_big_endian());
}

TEST(endian, to_big_endian)
{
    const bool system_is_be = is_system_big_endian();

    // 16-bit unsigned & signed
    uint16_t u16 = 0x1234;
    int16_t  s16 = -0x1234;
    EXPECT_EQ(hj::to_big_endian(u16),
              system_is_be ? u16 : static_cast<uint16_t>(0x3412));
    EXPECT_EQ(hj::to_big_endian(s16),
              system_is_be ? s16
                           : static_cast<int16_t>(hj::detail::bswap16(
                                 static_cast<uint16_t>(s16))));

    // 32-bit unsigned & signed
    uint32_t u32 = 0x01020304U;
    int32_t  s32 = -0x01020304;
    EXPECT_EQ(hj::to_big_endian(u32), system_is_be ? u32 : 0x04030201U);
    EXPECT_EQ(hj::to_big_endian(s32),
              system_is_be ? s32
                           : static_cast<int32_t>(hj::detail::bswap32(
                                 static_cast<uint32_t>(s32))));

    // 64-bit unsigned & signed
    uint64_t u64 = 0x0102030405060708ULL;
    int64_t  s64 = -0x0102030405060708LL;
    EXPECT_EQ(hj::to_big_endian(u64),
              system_is_be ? u64 : 0x0807060504030201ULL);
    EXPECT_EQ(hj::to_big_endian(s64),
              system_is_be ? s64
                           : static_cast<int64_t>(hj::detail::bswap64(
                                 static_cast<uint64_t>(s64))));
}

TEST(endian, to_little_endian)
{
    const bool system_is_be = is_system_big_endian();

    // 16-bit unsigned & signed
    uint16_t u16 = 0x1234;
    EXPECT_EQ(hj::to_little_endian(u16),
              system_is_be ? static_cast<uint16_t>(0x3412) : u16);

    // 32-bit unsigned & signed
    uint32_t u32 = 0x01020304U;
    EXPECT_EQ(hj::to_little_endian(u32), system_is_be ? 0x04030201U : u32);

    // 64-bit unsigned & signed
    uint64_t u64 = 0x0102030405060708ULL;
    EXPECT_EQ(hj::to_little_endian(u64),
              system_is_be ? 0x0807060504030201ULL : u64);
}

TEST(endian, edge_cases_and_round_trip)
{
    uint8_t u8 = 0xAB;
    int8_t  s8 = -12;
    EXPECT_EQ(hj::to_big_endian(u8), u8);
    EXPECT_EQ(hj::to_little_endian(s8), s8);

    EXPECT_EQ(hj::to_big_endian(static_cast<uint32_t>(0)), 0U);
    EXPECT_EQ(hj::to_big_endian(static_cast<uint64_t>(~0ULL)), ~0ULL);

    uint64_t original = 0xDEADBEEF12345678ULL;
    uint64_t be_val   = hj::to_big_endian(original);

    uint64_t restored = hj::to_big_endian(be_val);
    EXPECT_EQ(restored, original);

    uint64_t le_val    = hj::to_little_endian(original);
    uint64_t restored2 = hj::to_little_endian(le_val);
    EXPECT_EQ(restored2, original);
}