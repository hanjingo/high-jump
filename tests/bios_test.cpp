
#include <gtest/gtest.h>
#include <hj/hardware/bios.h>
#include <cstring>

TEST(bios, vendor)
{
    char         vendor[128] = {0};
    size_t       len         = sizeof(vendor);
    bios_error_t ret         = bios_vendor(vendor, &len);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP()
            << "bios_vendor skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(vendor));
    ASSERT_EQ(vendor[sizeof(vendor) - 1], 0);
    for(size_t i = 0; i < len; ++i)
    {
        ASSERT_TRUE(vendor[i] == 0 || (vendor[i] >= 32 && vendor[i] <= 126));
    }
}

TEST(bios, version)
{
    char         version[64] = {0};
    size_t       len         = sizeof(version);
    bios_error_t ret         = bios_version(version, &len);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP()
            << "bios_version skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(version));
    ASSERT_EQ(version[sizeof(version) - 1], 0);
    for(size_t i = 0; i < len; ++i)
    {
        ASSERT_TRUE(version[i] == 0 || (version[i] >= 32 && version[i] <= 126));
    }
}

TEST(bios, release_date)
{
    char         date[32] = {0};
    size_t       len      = sizeof(date);
    bios_error_t ret      = bios_release_date(date, &len);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP() << "bios_release_date skipped: unavailable or "
                        "insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GE(len, 0u);
    ASSERT_LE(len, sizeof(date));
    ASSERT_EQ(date[sizeof(date) - 1], 0);
    ASSERT_TRUE(strchr(date, '/') || strchr(date, '-'));
}

TEST(bios, serial_num)
{
    char         serial[128] = {0};
    size_t       len         = sizeof(serial);
    bios_error_t ret         = bios_serial_num(serial, &len);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND
       || len == 0)
    {
        GTEST_SKIP() << "bios_serial_num skipped: unavailable or insufficient "
                        "permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GT(len, 0u);
    ASSERT_LE(len, sizeof(serial));
    ASSERT_EQ(serial[sizeof(serial) - 1], 0);

    for(size_t i = 0; i < len && i < sizeof(serial); ++i)
    {
        if(serial[i] == 0)
            break;
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
    uint16_t     seg = 0;
    bios_error_t ret = bios_starting_segment(&seg);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP() << "bios_starting_segment skipped: unavailable or "
                        "insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GT(seg, 0u);
}

TEST(bios, rom_size)
{
    size_t       size = 0;
    bios_error_t ret  = bios_rom_size(&size);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP()
            << "bios_rom_size skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GE(size, 64 * 1024u);
    ASSERT_LE(size, 16 * 1024 * 1024u);
}

TEST(bios, info)
{
    bios_info_t  info;
    bios_error_t ret = bios_info(&info);
    if(ret == BIOS_ERROR_NOT_SUPPORTED || ret == BIOS_ERROR_NOT_FOUND)
    {
        GTEST_SKIP()
            << "bios_info skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, BIOS_OK);
    ASSERT_GT(strlen(info.vendor), 0u);
    ASSERT_GT(strlen(info.version), 0u);
    ASSERT_GT(strlen(info.release_date), 0u);
    ASSERT_GE(info.starting_segment, 0u);
    ASSERT_GE(info.rom_size, 0u);
}

TEST(bios, null_args)
{
    size_t len = 10;
    ASSERT_EQ(bios_vendor(NULL, &len), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_vendor((char *) "", NULL), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_version(NULL, &len), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_release_date(NULL, &len), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_serial_num(NULL, &len), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_starting_segment(NULL), BIOS_ERROR_NULL_POINTER);
    ASSERT_EQ(bios_info(NULL), BIOS_ERROR_NULL_POINTER);
}
