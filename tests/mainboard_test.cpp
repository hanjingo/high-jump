#include <gtest/gtest.h>
#include <hj/hardware/mainboard.h>

TEST(mainboard, model)
{
    char buf[256] = {0};
    int  ret      = mainboard_model(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_model skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}

TEST(mainboard, vendor)
{
    char buf[256] = {0};
    int  ret      = mainboard_vendor(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_vendor skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}

TEST(mainboard, serial_num)
{
    char buf[256] = {0};
    int  ret      = mainboard_serial_num(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_serial_num skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}

TEST(mainboard, bios_version)
{
    char buf[256] = {0};
    int  ret      = mainboard_bios_version(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_bios_version skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}

TEST(mainboard, chipset)
{
    char buf[256] = {0};
    int  ret      = mainboard_chipset(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_chipset skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}

TEST(mainboard, memory_slots)
{
    int ret = mainboard_memory_slots();
    EXPECT_GE(ret, -1);
}

TEST(mainboard, expansion_slots)
{
    int ret = mainboard_expansion_slots();
    EXPECT_GE(ret, -1);
}

TEST(mainboard, manufacturer_name)
{
    char buf[256] = {0};
    int  ret      = mainboard_manufacturer_name(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_manufacturer_name skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
    EXPECT_GT(strlen(buf), 0u);
}

TEST(mainboard, product_name)
{
    char buf[256] = {0};
    int  ret      = mainboard_product_name(buf, sizeof(buf));
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_product_name skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
    EXPECT_GT(strlen(buf), 0u);
}

TEST(mainboard, version)
{
    uint8_t major = 0, minor = 0, patch = 0;
    int     ret = mainboard_version(&major, &minor, &patch);
    if(ret != 0)
    {
        GTEST_SKIP() << "mainboard_version skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
}