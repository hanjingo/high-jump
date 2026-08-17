#include <gtest/gtest.h>

#define HJ_MAINBOARD_IMPL
#include <hj/hardware/mainboard.h>

TEST(mainboard, model)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret      = hj_mainboard_model(buf, sizeof(buf));

    if(ret == HJ_MAINBOARD_OK)
    {
        EXPECT_GT(strlen(buf), 0u);
    } else
    {
        EXPECT_TRUE(ret == HJ_MAINBOARD_ERR_READ_INFO_FAILED
                    || ret == HJ_MAINBOARD_ERR_SYSCTL_FAILED
                    || ret == HJ_MAINBOARD_ERR_NOT_SUPPORTED);
    }
}

TEST(mainboard, vendor)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret      = hj_mainboard_vendor(buf, sizeof(buf));

    if(ret == HJ_MAINBOARD_OK)
    {
        EXPECT_GT(strlen(buf), 0u);
#ifdef __APPLE__
        EXPECT_STREQ(buf, "Apple Inc.");
#endif
    } else
    {
        EXPECT_TRUE(ret == HJ_MAINBOARD_ERR_READ_INFO_FAILED
                    || ret == HJ_MAINBOARD_ERR_NOT_SUPPORTED);
    }
}

TEST(mainboard, serial_num)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret      = hj_mainboard_serial_num(buf, sizeof(buf));
    if(ret != HJ_MAINBOARD_OK)
    {
        GTEST_SKIP() << "hj_mainboard_serial_num skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_EQ(ret, HJ_MAINBOARD_OK);
}

TEST(mainboard, bios_version)
{
    uint8_t            major = 0, minor = 0, patch = 0;
    hj_mainboard_err_t ret = hj_mainboard_version(&major, &minor, &patch);

    if(ret == HJ_MAINBOARD_OK)
    {
        EXPECT_TRUE(major != 0 || minor != 0 || patch != 0);
    } else
    {
        EXPECT_TRUE(ret == HJ_MAINBOARD_ERR_NOT_SUPPORTED
                    || ret == HJ_MAINBOARD_ERR_READ_INFO_FAILED);
    }
}

TEST(mainboard, chipset)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret      = hj_mainboard_chipset(buf, sizeof(buf));
    if(ret == HJ_MAINBOARD_OK)
    {
        EXPECT_GT(strlen(buf), 0u);
        EXPECT_STRNE(buf, "Unknown");
    } else
    {
        EXPECT_EQ(ret, HJ_MAINBOARD_ERR_READ_INFO_FAILED);
        EXPECT_STREQ(buf, "Unknown");
    }
}

TEST(mainboard, memory_slots)
{
    unsigned int       slots = 0;
    hj_mainboard_err_t ret   = hj_mainboard_memory_slots(&slots);

    if(ret == HJ_MAINBOARD_OK)
    {
        EXPECT_GE(slots, 0u);
    } else
    {
        EXPECT_TRUE(ret == HJ_MAINBOARD_ERR_NOT_SUPPORTED
                    || ret == HJ_MAINBOARD_ERR_READ_INFO_FAILED
                    || ret == HJ_MAINBOARD_ERR_SYSCTL_FAILED);
    }
}

TEST(mainboard, expansion_slots)
{
    unsigned int       slots = 0;
    hj_mainboard_err_t ret   = hj_mainboard_expansion_slots(&slots);
    if(ret != HJ_MAINBOARD_OK)
    {
        GTEST_SKIP() << "hj_mainboard_expansion_slots skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_EQ(ret, HJ_MAINBOARD_OK);
}

TEST(mainboard, manufacturer_name)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret = hj_mainboard_manufacturer_name(buf, sizeof(buf));
    if(ret != HJ_MAINBOARD_OK)
    {
        GTEST_SKIP() << "hj_mainboard_manufacturer_name skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_EQ(ret, HJ_MAINBOARD_OK);
    EXPECT_GT(strlen(buf), 0u);
}

TEST(mainboard, product_name)
{
    char               buf[256] = {0};
    hj_mainboard_err_t ret      = hj_mainboard_product_name(buf, sizeof(buf));
    if(ret != HJ_MAINBOARD_OK)
    {
        GTEST_SKIP() << "hj_mainboard_product_name skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_EQ(ret, HJ_MAINBOARD_OK);
    EXPECT_GT(strlen(buf), 0u);
}

TEST(mainboard, version)
{
    uint8_t            major = 0, minor = 0, patch = 0;
    hj_mainboard_err_t ret = hj_mainboard_version(&major, &minor, &patch);
    if(ret != HJ_MAINBOARD_OK)
    {
        GTEST_SKIP() << "hj_mainboard_version skipped: insufficient "
                        "permissions or unavailable";
        return;
    }
    EXPECT_EQ(ret, HJ_MAINBOARD_OK);
}

TEST(mainboard, hj_parse_version_string)
{
    uint8_t major = 0, minor = 0, patch = 0;

    EXPECT_EQ(hj_parse_version_string("1.2.3", &major, &minor, &patch), 0);
    EXPECT_EQ(major, 1);
    EXPECT_EQ(minor, 2);
    EXPECT_EQ(patch, 3);

    EXPECT_EQ(hj_parse_version_string("v2.10.15-beta", &major, &minor, &patch),
              0);
    EXPECT_EQ(major, 2);
    EXPECT_EQ(minor, 10);
    EXPECT_EQ(patch, 15);

    EXPECT_EQ(hj_parse_version_string("3.4", &major, &minor, &patch), 0);
    EXPECT_EQ(major, 3);
    EXPECT_EQ(minor, 4);
    EXPECT_EQ(patch, 0);

    EXPECT_EQ(hj_parse_version_string("5", &major, &minor, &patch), 0);
    EXPECT_EQ(major, 5);
    EXPECT_EQ(minor, 0);
    EXPECT_EQ(patch, 0);

    EXPECT_EQ(hj_parse_version_string("255.255.255", &major, &minor, &patch),
              0);
    EXPECT_EQ(major, 255);
    EXPECT_EQ(minor, 255);
    EXPECT_EQ(patch, 255);
}

TEST(mainboard, hj_parse_version_string_overflow)
{
    uint8_t major = 0, minor = 0, patch = 0;

    EXPECT_NE(hj_parse_version_string("256.0.0", &major, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("1.300.0", &major, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("1.0.999", &major, &minor, &patch), 0);

    EXPECT_NE(hj_parse_version_string("Unknown", &major, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("", &major, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("...*", &major, &minor, &patch), 0);
}

TEST(mainboard, hj_parse_version_string_null_pointer_safety)
{
    uint8_t major = 0, minor = 0, patch = 0;

    EXPECT_NE(hj_parse_version_string(NULL, &major, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("1.0.0", NULL, &minor, &patch), 0);
    EXPECT_NE(hj_parse_version_string("1.0.0", &major, NULL, &patch), 0);
    EXPECT_NE(hj_parse_version_string("1.0.0", &major, &minor, NULL), 0);
}

TEST(mainboard, null_buffer_or_zero_size)
{
    char buf[256];

    EXPECT_EQ(hj_mainboard_model(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_model(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_vendor(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_vendor(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_serial_num(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_serial_num(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_bios_version(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_bios_version(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_chipset(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_chipset(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_manufacturer_name(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_manufacturer_name(buf, 0),
              HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_product_name(NULL, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_product_name(buf, 0), HJ_MAINBOARD_ERR_INVALID_ARG);

    EXPECT_EQ(hj_mainboard_memory_slots(NULL), HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_expansion_slots(NULL), HJ_MAINBOARD_ERR_INVALID_ARG);

    uint8_t ma, mi, pa;
    EXPECT_EQ(hj_mainboard_version(NULL, &mi, &pa),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_version(&ma, NULL, &pa),
              HJ_MAINBOARD_ERR_INVALID_ARG);
    EXPECT_EQ(hj_mainboard_version(&ma, &mi, NULL),
              HJ_MAINBOARD_ERR_INVALID_ARG);
}

#ifdef __APPLE__
TEST(mainboard, AppleVendorSmallBuffer)
{
    char buf[5] = {0};
    EXPECT_EQ(hj_mainboard_vendor(buf, sizeof(buf)),
              HJ_MAINBOARD_ERR_INVALID_ARG);
}
#endif