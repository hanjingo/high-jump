#include <sstream>
#include <gtest/gtest.h>
#include <hj/encoding/endian.hpp>

bool is_big_endian()
{
    int n = 1;
    return *(char *) (&n) == 0;
}


TEST(endian, is_big_endian)
{
    ASSERT_EQ(hj::is_big_endian(), is_big_endian());
}

TEST(endian, to_big_endian)
{
    uint16_t n16  = 0x1234;
    uint16_t be16 = hj::to_big_endian(n16);
    if(hj::is_big_endian())
        ASSERT_EQ(be16, 0x1234);
    else
        ASSERT_EQ(be16, 0x3412);

    int16_t s16    = 0x1234;
    int16_t be_s16 = hj::to_big_endian(s16);
    if(hj::is_big_endian())
        ASSERT_EQ(be_s16, 0x1234);
    else
        ASSERT_EQ(be_s16, 0x3412);

    uint32_t n32  = 0x01020304;
    uint32_t be32 = hj::to_big_endian(n32);
    if(hj::is_big_endian())
        ASSERT_EQ(be32, 0x01020304);
    else
        ASSERT_EQ(be32, 0x04030201);

    int32_t s32    = 0x01020304;
    int32_t be_s32 = hj::to_big_endian(s32);
    if(hj::is_big_endian())
        ASSERT_EQ(be_s32, 0x01020304);
    else
        ASSERT_EQ(be_s32, 0x04030201);

    uint64_t n64  = 0x0102030405060708ULL;
    uint64_t be64 = hj::to_big_endian(n64);
    if(hj::is_big_endian())
        ASSERT_EQ(be64, 0x0102030405060708ULL);
    else
        ASSERT_EQ(be64, 0x0807060504030201ULL);

    int64_t s64    = 0x0102030405060708LL;
    int64_t be_s64 = hj::to_big_endian(s64);
    if(hj::is_big_endian())
        ASSERT_EQ(be_s64, 0x0102030405060708LL);
    else
        ASSERT_EQ(be_s64, 0x0807060504030201LL);
}

TEST(endian, to_little_endian)
{
    uint16_t n16  = 0x1234;
    uint16_t le16 = hj::to_little_endian(n16);
    if(hj::is_big_endian())
        ASSERT_EQ(le16, 0x3412);
    else
        ASSERT_EQ(le16, 0x1234);

    int16_t s16    = 0x1234;
    int16_t le_s16 = hj::to_little_endian(s16);
    if(hj::is_big_endian())
        ASSERT_EQ(le_s16, 0x3412);
    else
        ASSERT_EQ(le_s16, 0x1234);

    uint32_t n32  = 0x01020304;
    uint32_t le32 = hj::to_little_endian(n32);
    if(hj::is_big_endian())
        ASSERT_EQ(le32, 0x04030201);
    else
        ASSERT_EQ(le32, 0x01020304);

    int32_t s32    = 0x01020304;
    int32_t le_s32 = hj::to_little_endian(s32);
    if(hj::is_big_endian())
        ASSERT_EQ(le_s32, 0x04030201);
    else
        ASSERT_EQ(le_s32, 0x01020304);

    uint64_t n64  = 0x0102030405060708ULL;
    uint64_t le64 = hj::to_little_endian(n64);
    if(hj::is_big_endian())
        ASSERT_EQ(le64, 0x0807060504030201ULL);
    else
        ASSERT_EQ(le64, 0x0102030405060708ULL);

    int64_t s64    = 0x0102030405060708LL;
    int64_t le_s64 = hj::to_little_endian(s64);
    if(hj::is_big_endian())
        ASSERT_EQ(le_s64, 0x0807060504030201LL);
    else
        ASSERT_EQ(le_s64, 0x0102030405060708LL);
}