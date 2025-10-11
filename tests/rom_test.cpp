#include <gtest/gtest.h>
#include <hj/hardware/rom.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

// Helper to create a test ROM file
static void create_test_rom(const char *filename, const char *content)
{
    FILE *fp = fopen(filename, "wb");
    if(!fp)
        FAIL() << "Failed to create test ROM file: " << filename;

    fwrite(content, 1, strlen(content), fp);
    fclose(fp);
}

class rom : public ::testing::Test
{
  protected:
    std::string test_file;
    std::string test_content;
    rom_t       _rom;

    void SetUp() override
    {
        auto cwd     = std::filesystem::current_path();
        test_file    = (cwd / "test.rom").string();
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    void TearDown() override
    {
        rom_free(&_rom);
        remove(test_file.c_str());
    }
};

// Test ROM initialization
TEST_F(rom, initialization)
{
    rom_t r;
    rom_init(&r);
    EXPECT_EQ(r.data, nullptr);
    EXPECT_EQ(r.size, 0u);
    EXPECT_FALSE(r.loaded);
}

// Test loading ROM from file
TEST_F(rom, load)
{
    if(!std::filesystem::exists(test_file))
    {
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    EXPECT_TRUE(rom_load(&_rom, test_file.c_str()));
    EXPECT_TRUE(_rom.loaded);
    EXPECT_EQ(_rom.size, test_content.size());
}

// Test reading from ROM
TEST_F(rom, read)
{
    if(!std::filesystem::exists(test_file))
    {
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    ASSERT_TRUE(rom_load(&_rom, test_file.c_str()));
    char   buf[16] = {0};
    size_t n       = rom_read(&_rom, 0, buf, 4);
    EXPECT_EQ(n, 4u);
    EXPECT_EQ(std::string(buf, 4), "ROMD");

    n = rom_read(&_rom, 5, buf, 16);
    EXPECT_EQ(n, test_content.size() - 5);
    EXPECT_EQ(std::string(buf, n), "TA123");
}

// Test reading with offset out of range
TEST_F(rom, read_offset_out_of_range)
{
    if(!std::filesystem::exists(test_file))
    {
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    ASSERT_TRUE(rom_load(&_rom, test_file.c_str()));
    char   buf[8] = {0};
    size_t n      = rom_read(&_rom, 100, buf, 4);
    EXPECT_EQ(n, 0u);
}

// Test loading with invalid arguments
TEST_F(rom, load_invalid_args)
{
    if(!std::filesystem::exists(test_file))
    {
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    EXPECT_FALSE(rom_load(nullptr, test_file.c_str()));
    EXPECT_FALSE(rom_load(&_rom, nullptr));
}

// Test freeing ROM
TEST_F(rom, free)
{
    if(!std::filesystem::exists(test_file))
    {
        test_content = "ROMDATA123";
        create_test_rom(test_file.c_str(), test_content.c_str());
        rom_init(&_rom);
    }

    ASSERT_TRUE(rom_load(&_rom, test_file.c_str()));
    rom_free(&_rom);
    EXPECT_EQ(_rom.data, nullptr);
    EXPECT_EQ(_rom.size, 0u);
    EXPECT_FALSE(_rom.loaded);
}
