#define HJ_BIOS_IMPL
#include <hj/hardware/bios.h>

#include <gtest/gtest.h>
#include <cstring>
#include <vector>

TEST(hj_bios_safe_string_copy, NormalCopy)
{
    char          buf[16] = {0};
    hj_bios_err_t ret     = hj_bios_safe_string_copy(buf, sizeof(buf), "Hello");
    EXPECT_EQ(ret, HJ_BIOS_OK);
    EXPECT_STREQ(buf, "Hello");
}

TEST(hj_bios_safe_string_copy, ExactFit)
{
    char          buf[6] = {0};
    hj_bios_err_t ret    = hj_bios_safe_string_copy(buf, sizeof(buf), "12345");
    EXPECT_EQ(ret, HJ_BIOS_OK);
    EXPECT_STREQ(buf, "12345");
}

TEST(hj_bios_safe_string_copy, BufferTooSmall)
{
    char          buf[5] = {0};
    hj_bios_err_t ret    = hj_bios_safe_string_copy(buf, sizeof(buf), "12345");
    EXPECT_EQ(ret, HJ_BIOS_ERROR_BUFFER_TOO_SMALL);
}

TEST(hj_bios_safe_string_copy, NullPointer)
{
    char buf[16] = {0};
    EXPECT_EQ(hj_bios_safe_string_copy(NULL, sizeof(buf), "test"),
              HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_safe_string_copy(buf, sizeof(buf), NULL),
              HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_safe_string_copy(NULL, 0, NULL),
              HJ_BIOS_ERROR_NULL_POINTER);
}

TEST(hj_bios_safe_string_copy, ZeroLengthDst)
{
    char buf[16] = {0};
    EXPECT_EQ(hj_bios_safe_string_copy(buf, 0, "test"),
              HJ_BIOS_ERROR_NULL_POINTER);
}

TEST(hj_bios_null_args, AllFunctions)
{
    char     buf[128] = {0};
    size_t   len      = sizeof(buf);
    size_t   zero_len = 0;
    uint16_t seg      = 0;
    size_t   rom_sz   = 0;

    // bios_vendor
    EXPECT_EQ(hj_bios_vendor(NULL, &len), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_vendor(buf, NULL), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_vendor(buf, &zero_len), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_version
    EXPECT_EQ(hj_bios_version(NULL, &len), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_version(buf, NULL), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_version(buf, &zero_len), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_release_date
    EXPECT_EQ(hj_bios_release_date(NULL, &len), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_release_date(buf, NULL), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_release_date(buf, &zero_len), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_serial_num
    EXPECT_EQ(hj_bios_serial_num(NULL, &len), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_serial_num(buf, NULL), HJ_BIOS_ERROR_NULL_POINTER);
    EXPECT_EQ(hj_bios_serial_num(buf, &zero_len), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_starting_segment
    EXPECT_EQ(hj_bios_starting_segment(NULL), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_rom_size
    EXPECT_EQ(hj_bios_rom_size(NULL), HJ_BIOS_ERROR_NULL_POINTER);

    // bios_info
    EXPECT_EQ(hj_bios_info(NULL), HJ_BIOS_ERROR_NULL_POINTER);
}

TEST(hj_bios_buffer_too_small, TinyBuffer)
{
    char tiny_buf[1] = {0};

    size_t        len = sizeof(tiny_buf);
    hj_bios_err_t ret = hj_bios_vendor(tiny_buf, &len);
    if(ret != HJ_BIOS_ERROR_NOT_SUPPORTED && ret != HJ_BIOS_ERROR_NOT_FOUND)
    {
        EXPECT_EQ(ret, HJ_BIOS_ERROR_BUFFER_TOO_SMALL);
    }

    len = sizeof(tiny_buf);
    ret = hj_bios_version(tiny_buf, &len);
    if(ret != HJ_BIOS_ERROR_NOT_SUPPORTED && ret != HJ_BIOS_ERROR_NOT_FOUND)
    {
        EXPECT_EQ(ret, HJ_BIOS_ERROR_BUFFER_TOO_SMALL);
    }

    len = sizeof(tiny_buf);
    ret = hj_bios_serial_num(tiny_buf, &len);
    if(ret != HJ_BIOS_ERROR_NOT_SUPPORTED && ret != HJ_BIOS_ERROR_NOT_FOUND)
    {
        EXPECT_EQ(ret, HJ_BIOS_ERROR_BUFFER_TOO_SMALL);
    }
}

TEST(bios, vendor)
{
    char          vendor[128] = {0};
    size_t        len         = sizeof(vendor);
    hj_bios_err_t ret         = hj_bios_vendor(vendor, &len);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP()
            << "bios_vendor skipped: unavailable or insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(vendor));
    ASSERT_EQ(vendor[sizeof(vendor) - 1], 0);
    ASSERT_EQ(strlen(vendor), len);

    for(size_t i = 0; i < len; ++i)
    {
        ASSERT_TRUE(vendor[i] == 0 || (vendor[i] >= 32 && vendor[i] <= 126));
    }
}

TEST(bios, version)
{
    char          version[64] = {0};
    size_t        len         = sizeof(version);
    hj_bios_err_t ret         = hj_bios_version(version, &len);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP()
            << "bios_version skipped: unavailable or insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(version));
    ASSERT_EQ(version[sizeof(version) - 1], 0);
    ASSERT_EQ(strlen(version), len);

    for(size_t i = 0; i < len; ++i)
    {
        ASSERT_TRUE(version[i] == 0 || (version[i] >= 32 && version[i] <= 126));
    }
}

TEST(bios, release_date)
{
    char          date[32] = {0};
    size_t        len      = sizeof(date);
    hj_bios_err_t ret      = hj_bios_release_date(date, &len);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP() << "bios_release_date skipped: unavailable or "
                        "insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_LE(len, sizeof(date));
    ASSERT_EQ(date[sizeof(date) - 1], 0);

    if(len > 0)
    {
        ASSERT_EQ(strlen(date), len);
        ASSERT_TRUE(strchr(date, '/') || strchr(date, '-')
                    || strchr(date, '.'));
    }
}

TEST(bios, serial_num)
{
    char          serial[128] = {0};
    size_t        len         = sizeof(serial);
    hj_bios_err_t ret         = hj_bios_serial_num(serial, &len);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP() << "bios_serial_num skipped: unavailable or insufficient "
                        "permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(serial));
    ASSERT_EQ(serial[sizeof(serial) - 1], 0);
    ASSERT_EQ(strlen(serial), len);

    for(size_t i = 0; i < len; ++i)
    {
        if(serial[i] < 32 || serial[i] > 126)
        {
            GTEST_SKIP() << "serial_num contains non-printable char: 0x"
                         << std::hex << (int) serial[i];
            return;
        }
    }
}

TEST(bios, starting_segment)
{
    uint16_t      seg = 0;
    hj_bios_err_t ret = hj_bios_starting_segment(&seg);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP() << "bios_starting_segment skipped: unavailable or "
                        "insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    if(seg == 0)
    {
        GTEST_SKIP() << "bios_starting_segment skipped: segment is 0 (UEFI or "
                        "unsupported)";
        return;
    }
    ASSERT_GT(seg, 0u);
}

TEST(bios, rom_size)
{
    size_t        size = 0;
    hj_bios_err_t ret  = hj_bios_rom_size(&size);

    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP()
            << "bios_rom_size skipped: unavailable or insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_GE(size, 64 * 1024u);
    ASSERT_LE(size, 128 * 1024 * 1024u);
}

TEST(bios, info)
{
    hj_bios_info_t info;
    memset(&info, 0xFF, sizeof(info));

    hj_bios_err_t ret = hj_bios_info(&info);
    if(ret == HJ_BIOS_ERROR_NOT_SUPPORTED || ret == HJ_BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP()
            << "bios_info skipped: unavailable or insufficient permissions";
        return;
    }

    ASSERT_EQ(ret, HJ_BIOS_OK);
    ASSERT_EQ(info.vendor[sizeof(info.vendor) - 1], '\0');
    ASSERT_EQ(info.version[sizeof(info.version) - 1], '\0');
    ASSERT_EQ(info.release_date[sizeof(info.release_date) - 1], '\0');
    ASSERT_EQ(info.serial_number[sizeof(info.serial_number) - 1], '\0');
}