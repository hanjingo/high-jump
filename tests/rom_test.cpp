#include <gtest/gtest.h>
#include <libcpp/hardware/rom.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

// Helper to create a test ROM file
static void create_test_rom(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) 
        FAIL() << "Failed to create test ROM file: " << filename;

    fwrite(content, 1, strlen(content), fp);
    fclose(fp);
}

class RomTest : public ::testing::Test {
protected:
    std::string test_file;
    std::string test_content;
    rom_t rom;

    void SetUp() override {
        auto cwd = std::filesystem::current_path();
        test_file = (cwd / "test.rom").string();
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&rom);
    }

    void TearDown() override {
        rom_free(&rom);
        remove(test_file.c_str());
    }
};

// Test ROM initialization
TEST_F(RomTest, Initialization) {
    rom_t r;
    rom_init(&r);
    EXPECT_EQ(r.data, nullptr);
    EXPECT_EQ(r.size, 0u);
    EXPECT_FALSE(r.loaded);
}

// Test loading ROM from file
TEST_F(RomTest, Load) {
    EXPECT_TRUE(rom_load(&rom, test_file.c_str()));
    EXPECT_TRUE(rom.loaded);
    EXPECT_EQ(rom.size, test_content.size());
}

// Test reading from ROM
TEST_F(RomTest, Read) {
    ASSERT_TRUE(rom_load(&rom, test_file.c_str()));
    char buf[16] = {0};
    size_t n = rom_read(&rom, 0, buf, 4);
    EXPECT_EQ(n, 4u);
    EXPECT_EQ(std::string(buf, 4), "ROMD");

    n = rom_read(&rom, 5, buf, 16);
    EXPECT_EQ(n, test_content.size() - 5);
    EXPECT_EQ(std::string(buf, n), "TA123");
}

// Test reading with offset out of range
TEST_F(RomTest, ReadOffsetOutOfRange) {
    ASSERT_TRUE(rom_load(&rom, test_file.c_str()));
    char buf[8] = {0};
    size_t n = rom_read(&rom, 100, buf, 4);
    EXPECT_EQ(n, 0u);
}

// Test reading with null buffer
TEST_F(RomTest, ReadNullBuffer) {
    ASSERT_TRUE(rom_load(&rom, test_file.c_str()));
    size_t n = rom_read(&rom, 0, nullptr, 4);
    EXPECT_EQ(n, 0u);
}

// Test loading with invalid arguments
TEST_F(RomTest, LoadInvalidArgs) {
    EXPECT_FALSE(rom_load(nullptr, test_file.c_str()));
    EXPECT_FALSE(rom_load(&rom, nullptr));
}

// Test freeing ROM
TEST_F(RomTest, Free) {
    ASSERT_TRUE(rom_load(&rom, test_file.c_str()));
    rom_free(&rom);
    EXPECT_EQ(rom.data, nullptr);
    EXPECT_EQ(rom.size, 0u);
    EXPECT_FALSE(rom.loaded);
}

