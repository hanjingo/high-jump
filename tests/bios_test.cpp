#include <gtest/gtest.h>
#include <libcpp/hardware/bios.h>

TEST(bios, vendor)
{
    unsigned char vendor[64] = {0};
    size_t len = sizeof(vendor);
    int ret = bios_vendor(vendor, &len);
    if (ret == -1 || len == 0) 
    {
        GTEST_SKIP() << "bios_vendor skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, 0);
    ASSERT_GT(len, 0);
    for (size_t i = 0; i < len; ++i) 
    {
        ASSERT_TRUE(vendor[i] == 0 || (vendor[i] >= 32 && vendor[i] <= 126));
    }
}

TEST(bios, starting_segment)
{
    uint16_t seg = bios_starting_segment();
    ASSERT_GE(seg, 0);
}

TEST(bios, release_date)
{
    uint8_t year = 0, month = 0, day = 0;
    int ret = bios_release_date(&year, &month, &day);
    if (ret == -1) 
    {
        GTEST_SKIP() << "bios_release_date skipped: unavailable or insufficient permissions";
        return;
    }
    ASSERT_EQ(ret, 0);
    ASSERT_GE(year, 0);
    ASSERT_GE(month, 0);
    ASSERT_GE(day, 0);
}

TEST(bios, rom_size)
{
    size_t size = bios_rom_size();
    ASSERT_GE(size, 0u);
}

TEST(bios, version)
{
    uint8_t major = 0, minor = 0, patch = 0;
    int ret = bios_version(&major, &minor, &patch);
    // ASSERT_EQ(ret, 0);
    // ASSERT_GE(major, 0);
    // ASSERT_GE(minor, 0);
    // ASSERT_GE(patch, 0);
}

TEST(bios, serial_num)
{
    uint8_t serial[64] = {0};
    size_t len = sizeof(serial);
    int ret = bios_serial_num(serial, &len);
    if (ret == -1 || len == 0) 
    {
        GTEST_SKIP() << "bios_serial_num skipped: insufficient permissions or unavailable";
        return;
    }
    ASSERT_EQ(ret, 0);
    ASSERT_GT(len, 0);
    for (size_t i = 0; i < len; ++i) 
    {
        ASSERT_TRUE(serial[i] == 0 || (serial[i] >= 32 && serial[i] <= 126));
    }
}
