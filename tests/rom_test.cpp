#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

#define HJ_ROM_IMPL
#include <hj/hardware/rom.h>

static void
create_test_rom(const std::string &filename, const char *content, size_t len)
{
    FILE *fp = fopen(filename.c_str(), "wb");
    if(!fp)
        FAIL() << "Failed to create test ROM file: " << filename;

    fwrite(content, 1, len, fp);
    fclose(fp);
}

static void create_empty_rom(const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "wb");
    if(fp)
    {
        fclose(fp);
    }
}

TEST(rom, initialization)
{
    rom_t r;
    rom_init(&r);
    EXPECT_EQ(r.data, nullptr);
    EXPECT_EQ(r.size, 0u);
    EXPECT_FALSE(r.loaded);

    rom_init(nullptr);
}

TEST(rom, load_success)
{
    std::string test_content = "ROMDATA123456789";
    std::string test_file =
        (std::filesystem::current_path() / "test_success.rom").string();

    std::filesystem::remove_all(test_file);
    create_test_rom(test_file, test_content.data(), test_content.size());

    if(!std::filesystem::exists(test_file))
    {
        GTEST_SKIP() << "skip test rom::load_success create file failed";
    }

    rom_t _rom;
    rom_init(&_rom);

    EXPECT_EQ(rom_load(&_rom, test_file.c_str()), ROM_SUCCESS);
    EXPECT_TRUE(_rom.loaded);
    EXPECT_NE(_rom.data, nullptr);
    EXPECT_EQ(_rom.size, test_content.size());

    rom_free(&_rom);
    std::filesystem::remove_all(test_file);
}

TEST(rom, load_error_conditions)
{
    rom_t _rom;
    rom_init(&_rom);

    EXPECT_EQ(rom_load(nullptr, "dummy.rom"), ROM_ERR_INVALID_PARAM);
    EXPECT_EQ(rom_load(&_rom, nullptr), ROM_ERR_INVALID_PARAM);

    std::string non_existent =
        (std::filesystem::current_path() / "non_existent.rom").string();
    std::filesystem::remove_all(non_existent);
    EXPECT_EQ(rom_load(&_rom, non_existent.c_str()), ROM_ERR_FILE_NOT_FOUND);

    std::string empty_file =
        (std::filesystem::current_path() / "empty.rom").string();
    std::filesystem::remove_all(empty_file);
    create_empty_rom(empty_file);
    if(std::filesystem::exists(empty_file))
    {
        EXPECT_EQ(rom_load(&_rom, empty_file.c_str()), ROM_ERR_EMPTY_FILE);
        std::filesystem::remove_all(empty_file);
    }
}

TEST(rom, read_operations)
{
    std::string test_content = "HELLO_ROM_WORLD";
    std::string test_file =
        (std::filesystem::current_path() / "test_read.rom").string();

    std::filesystem::remove_all(test_file);
    create_test_rom(test_file, test_content.data(), test_content.size());

    rom_t _rom;
    rom_init(&_rom);
    ASSERT_EQ(rom_load(&_rom, test_file.c_str()), ROM_SUCCESS);

    char   buf[32]  = {0};
    size_t out_read = 0;

    EXPECT_EQ(rom_read(&_rom, 0, buf, 5, &out_read), ROM_SUCCESS);
    EXPECT_EQ(out_read, 5u);
    EXPECT_EQ(std::string(buf, 5), "HELLO");

    memset(buf, 0, sizeof(buf));
    EXPECT_EQ(rom_read(&_rom, 0, buf, 0, &out_read), ROM_SUCCESS);
    EXPECT_EQ(out_read, 0u);

    EXPECT_EQ(rom_read(&_rom, 100, buf, 4, &out_read), ROM_ERR_OUT_OF_BOUNDS);

    memset(buf, 0, sizeof(buf));
    size_t offset = test_content.size() - 5;
    EXPECT_EQ(rom_read(&_rom, offset, buf, 20, &out_read), ROM_SUCCESS);
    EXPECT_EQ(out_read, 5u);
    EXPECT_EQ(std::string(buf, 5), "WORLD");

    rom_free(&_rom);
    std::filesystem::remove_all(test_file);
}

TEST(rom, read_invalid_state)
{
    char  buf[16];
    rom_t _rom;
    rom_init(&_rom);
    size_t out_read = 0;

    EXPECT_EQ(rom_read(&_rom, 0, buf, sizeof(buf), &out_read),
              ROM_ERR_NOT_LOADED);

    EXPECT_EQ(rom_read(nullptr, 0, buf, sizeof(buf), &out_read),
              ROM_ERR_INVALID_PARAM);

    std::string test_file =
        (std::filesystem::current_path() / "test_dummy.rom").string();
    create_test_rom(test_file, "1234", 4);
    ASSERT_EQ(rom_load(&_rom, test_file.c_str()), ROM_SUCCESS);

    EXPECT_EQ(rom_read(&_rom, 0, nullptr, 4, &out_read), ROM_ERR_INVALID_PARAM);

    rom_free(&_rom);
    std::filesystem::remove_all(test_file);
}

TEST(rom, free_operations)
{
    std::string test_file =
        (std::filesystem::current_path() / "test_free.rom").string();
    create_test_rom(test_file, "DATA", 4);

    rom_t _rom;
    rom_init(&_rom);
    ASSERT_EQ(rom_load(&_rom, test_file.c_str()), ROM_SUCCESS);

    rom_free(&_rom);
    EXPECT_EQ(_rom.data, nullptr);
    EXPECT_EQ(_rom.size, 0u);
    EXPECT_FALSE(_rom.loaded);

    rom_free(&_rom);
    rom_free(nullptr);

    std::filesystem::remove_all(test_file);
}

TEST(rom, transactional_reload)
{
    std::string file1 =
        (std::filesystem::current_path() / "file1.rom").string();
    std::string file2 =
        (std::filesystem::current_path() / "file2.rom").string();
    std::string non_existent =
        (std::filesystem::current_path() / "bad.rom").string();
    std::filesystem::remove_all(non_existent);

    create_test_rom(file1, "FIRST_ROM", 9);
    create_test_rom(file2, "SECOND_ROM_EXTENDED", 19);

    rom_t _rom;
    rom_init(&_rom);

    ASSERT_EQ(rom_load(&_rom, file1.c_str()), ROM_SUCCESS);
    EXPECT_EQ(_rom.size, 9u);

    EXPECT_NE(rom_load(&_rom, non_existent.c_str()), ROM_SUCCESS);
    EXPECT_TRUE(_rom.loaded);
    EXPECT_EQ(_rom.size, 9u);

    char   buf[32]  = {0};
    size_t out_read = 0;
    EXPECT_EQ(rom_read(&_rom, 0, buf, 5, &out_read), ROM_SUCCESS);
    EXPECT_EQ(std::string(buf, 5), "FIRST");

    EXPECT_EQ(rom_load(&_rom, file2.c_str()), ROM_SUCCESS);
    EXPECT_EQ(_rom.size, 19u);
    memset(buf, 0, sizeof(buf));
    EXPECT_EQ(rom_read(&_rom, 0, buf, 6, &out_read), ROM_SUCCESS);
    EXPECT_EQ(std::string(buf, 6), "SECOND");

    rom_free(&_rom);
    std::filesystem::remove_all(file1);
    std::filesystem::remove_all(file2);
}