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

// Test ROM initialization
TEST(rom, initialization)
{
    rom_t r;
    rom_init(&r);
    EXPECT_EQ(r.data, nullptr);
    EXPECT_EQ(r.size, 0u);
    EXPECT_FALSE(r.loaded);
}

// Test loading ROM from file
TEST(rom, load)
{
    rom_t       _rom;
    std::string test_content = "ROMDATA123";
    std::string test_file = (std::filesystem::current_path() / "test.rom").string();

    std::filesystem::remove_all(test_file);
    create_test_rom(test_file.c_str(), test_content.c_str());
    rom_init(&_rom);
    if(!std::filesystem::exists(test_file))
    {
        GTEST_SKIP() << "skip test rom::load create file failed";
    }

    EXPECT_TRUE(rom_load(&_rom, test_file.c_str()));
    EXPECT_TRUE(_rom.loaded);
    EXPECT_EQ(_rom.size, test_content.size());
}

// Test reading from ROM
TEST(rom, read)
{
    rom_t       _rom;
    std::string test_content = "ROMDATA123";
    std::string test_file = (std::filesystem::current_path() / "test.rom").string();
    
    std::filesystem::remove_all(test_file);
    create_test_rom(test_file.c_str(), test_content.c_str());
    rom_init(&_rom);
    if(!std::filesystem::exists(test_file))
    {
        GTEST_SKIP() << "skip test rom::read create file failed";
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
TEST(rom, read_offset_out_of_range)
{
    rom_t       _rom;
    std::string test_content = "ROMDATA123";
    std::string test_file = (std::filesystem::current_path() / "test.rom").string();
    
    std::filesystem::remove_all(test_file);
    create_test_rom(test_file.c_str(), test_content.c_str());
    rom_init(&_rom);
    if(!std::filesystem::exists(test_file))
    {
        GTEST_SKIP() << "skip test rom::read_offset_out_of_range create file failed";
    }

    ASSERT_TRUE(rom_load(&_rom, test_file.c_str()));
    char   buf[8] = {0};
    size_t n      = rom_read(&_rom, 100, buf, 4);
    EXPECT_EQ(n, 0u);
}

// Test loading with invalid arguments
TEST(rom, load_invalid_args)
{
    rom_t       _rom;
    std::string test_content = "ROMDATA123";
    std::string test_file = (std::filesystem::current_path() / "test.rom").string();
    
    std::filesystem::remove_all(test_file);
    create_test_rom(test_file.c_str(), test_content.c_str());
    rom_init(&_rom);
    if(!std::filesystem::exists(test_file))
    {
        GTEST_SKIP() << "skip test rom::load_invalid_args create file failed";
    }

    EXPECT_FALSE(rom_load(nullptr, test_file.c_str()));
    EXPECT_FALSE(rom_load(&_rom, nullptr));
}

// // Test freeing ROM
// TEST(rom, free)
// {
//     rom_t       _rom;
//     std::string test_content = "ROMDATA123";
//     std::string test_file = (std::filesystem::current_path() / "test.rom").string();
    
//     std::filesystem::remove_all(test_file);
//     create_test_rom(test_file.c_str(), test_content.c_str());
//     rom_init(&_rom);
//     if(!std::filesystem::exists(test_file))
//     {
//         GTEST_SKIP() << "skip test rom::free create file failed";
//     }

//     ASSERT_TRUE(rom_load(&_rom, test_file.c_str()));
//     rom_free(&_rom);
//     EXPECT_EQ(_rom.data, nullptr);
//     EXPECT_EQ(_rom.size, 0u);
//     EXPECT_FALSE(_rom.loaded);
// }
