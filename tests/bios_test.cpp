#include <gtest/gtest.h>
#include <libcpp/hardware/bios.h>

TEST(bios, version)
{
    uint8_t major = 0, minor = 0, patch = 0;
    int ret = bios_version(&major, &minor, &patch);
    ASSERT_EQ(ret, 0);

    ASSERT_GE(major, 0);
    ASSERT_GE(minor, 0);
    ASSERT_GE(patch, 0);
}

TEST(bios, serial_num)
{
    uint8_t serial[64] = {0};
    size_t len = sizeof(serial);
    int ret = bios_serial_num(serial, &len);

    if (ret == -1) {
        GTEST_SKIP() << "bios_serial_num skipped: insufficient permissions or unavailable";
        return;
    }

    ASSERT_EQ(ret, 0);
    ASSERT_GT(len, 0);

    for (size_t i = 0; i < len; ++i) {
        ASSERT_TRUE(serial[i] == 0 || (serial[i] >= 32 && serial[i] <= 126));
    }
}
