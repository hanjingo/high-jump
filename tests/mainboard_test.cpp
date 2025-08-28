#include <gtest/gtest.h>
#include <libcpp/hardware/mainboard.h>

TEST(MainboardTest, Model)
{
	char buf[256] = {0};
	int ret = mainboard_model(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	printf("mainboard_model: ret=%d, value='%s'\n", ret, buf);
}

TEST(MainboardTest, Vendor)
{
	char buf[256] = {0};
	int ret = mainboard_vendor(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	printf("mainboard_vendor: ret=%d, value='%s'\n", ret, buf);
}

TEST(MainboardTest, SerialNum)
{
    char buf[256] = {0};
    int ret = mainboard_serial_num(buf, sizeof(buf));
    if (ret == -1) {
        GTEST_SKIP() << "mainboard_serial_num skipped: insufficient permissions or unavailable";
        return;
    }
    EXPECT_GE(ret, 0);
    printf("mainboard_serial_num: ret=%d, value='%s'\n", ret, buf);
}

TEST(MainboardTest, BiosVersion)
{
	char buf[256] = {0};
	int ret = mainboard_bios_version(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	printf("mainboard_bios_version: ret=%d, value='%s'\n", ret, buf);
}

TEST(MainboardTest, Chipset)
{
	char buf[256] = {0};
	int ret = mainboard_chipset(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	printf("mainboard_chipset: ret=%d, value='%s'\n", ret, buf);
}

TEST(MainboardTest, MemorySlots)
{
	int ret = mainboard_memory_slots();
	EXPECT_GE(ret, -1);
	printf("mainboard_memory_slots: ret=%d\n", ret);
}

TEST(MainboardTest, ExpansionSlots)
{
	int ret = mainboard_expansion_slots();
	EXPECT_GE(ret, -1);
	printf("mainboard_expansion_slots: ret=%d\n", ret);
}
