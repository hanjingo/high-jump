#include <gtest/gtest.h>
#include <libcpp/hardware/mainboard.h>

TEST(MainboardTest, Model)
{
	char buf[256] = {0};
	int ret = mainboard_model(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
}

TEST(MainboardTest, Vendor)
{
	char buf[256] = {0};
	int ret = mainboard_vendor(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
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
}

TEST(MainboardTest, BiosVersion)
{
	char buf[256] = {0};
	int ret = mainboard_bios_version(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
}

TEST(MainboardTest, Chipset)
{
	char buf[256] = {0};
	int ret = mainboard_chipset(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
}

TEST(MainboardTest, MemorySlots)
{
	int ret = mainboard_memory_slots();
	EXPECT_GE(ret, -1);
}

TEST(MainboardTest, ExpansionSlots)
{
	int ret = mainboard_expansion_slots();
	EXPECT_GE(ret, -1);
}

TEST(MainboardTest, ManufacturerName)
{
	char buf[256] = {0};
	int ret = mainboard_manufacturer_name(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	EXPECT_GT(strlen(buf), 0u);
}

TEST(MainboardTest, ProductName)
{
	char buf[256] = {0};
	int ret = mainboard_product_name(buf, sizeof(buf));
	EXPECT_GE(ret, 0);
	EXPECT_GT(strlen(buf), 0u);
}

TEST(MainboardTest, Version)
{
	uint8_t major = 0, minor = 0, patch = 0;
	int ret = mainboard_version(&major, &minor, &patch);
	EXPECT_GE(ret, 0);
}