#include <gtest/gtest.h>
#include <hj/os/dll.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

typedef int (*hello_fn_t)(void);
typedef int (*world_fn_t)(void);

namespace fs = std::filesystem;

static std::string get_test_dll_path()
{
    return std::string("./") + DLL_PREFIX + "dll_example" + DLL_EXT;
}

TEST(dll, open_and_close)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr) << "Failed to open: " << dll_pop_error();
    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll, repeated_open)
{
    std::string dll_file = get_test_dll_path();

    void *handle_a = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle_a, nullptr) << "Error: " << dll_pop_error();

    void *handle_b = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle_b, nullptr) << "Error: " << dll_pop_error();

    auto fn1 = (hello_fn_t) dll_get(handle_b, "hello");
    ASSERT_NE(fn1, nullptr) << "Error: " << dll_pop_error();
    EXPECT_EQ(fn1(), 1);

    EXPECT_TRUE(dll_close(handle_a) == 0);
    EXPECT_TRUE(dll_close(handle_b) == 0);
}

TEST(dll, get_and_call_symbol)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr) << "Error: " << dll_pop_error();

    hello_fn_t fn_hello = (hello_fn_t) dll_get(handle, "hello");
    ASSERT_NE(fn_hello, nullptr) << "Error: " << dll_pop_error();
    EXPECT_EQ(fn_hello(), 1);

    world_fn_t fn_world = (world_fn_t) dll_get(handle, "world");
    ASSERT_NE(fn_world, nullptr) << "Error: " << dll_pop_error();
    EXPECT_EQ(fn_world(), 2);

    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll, invalid_symbol_queries)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr);

    void *invalid_fn = dll_get(handle, "this_symbol_does_not_exist");
    EXPECT_EQ(invalid_fn, nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    void *null_sym = dll_get(handle, NULL);
    EXPECT_EQ(null_sym, nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    EXPECT_EQ(dll_get(NULL, "hello"), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll, rtld_noload_semantics)
{
    void *probe_not_loaded =
        dll_open("./not_exist_module_for_test.dll", DLL_MODE_RTLD_NOLOAD);
    EXPECT_EQ(probe_not_loaded, nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    std::string dll_file = get_test_dll_path();

    void *real_handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(real_handle, nullptr) << "Error: " << dll_pop_error();

    void *noload_handle = dll_open(dll_file.c_str(), DLL_MODE_RTLD_NOLOAD);
    EXPECT_NE(noload_handle, nullptr);

    if(noload_handle)
    {
        dll_close(noload_handle);
    }

    EXPECT_TRUE(dll_close(real_handle) == 0);
}

TEST(dll, invalid_inputs)
{
    EXPECT_EQ(dll_open(NULL, DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    EXPECT_EQ(dll_open("./not_exist_library.so", DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    EXPECT_FALSE(dll_close(NULL) == 0);
    EXPECT_STRNE(dll_pop_error(), "");
}

TEST(dll, error_handling_and_clearing)
{
    dll_clear_error();
    EXPECT_STREQ(dll_pop_error(), "");

    dll_open(NULL, DLL_MODE_DEFAULT);
    EXPECT_STRNE(dll_pop_error(), "");

    dll_clear_error();
    EXPECT_STREQ(dll_pop_error(), "");
}

TEST(dll, multithreaded_error_isolation)
{
    dll_clear_error();

    std::thread t([]() {
        dll_open(NULL, DLL_MODE_DEFAULT);
        EXPECT_STRNE(dll_pop_error(), "");
    });

    t.join();

    EXPECT_STREQ(dll_pop_error(), "");
}

TEST(dll, long_path_and_utf8)
{
    std::string long_path = "./";
    for(int i = 0; i < 30; ++i)
    {
        long_path += "very_long_path_component_";
    }
    long_path += DLL_EXT;

    EXPECT_EQ(dll_open(long_path.c_str(), DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

#if defined(_WIN32)
    const char invalid_utf8[] = {'\xC0', '\xAF', '\0'};
    EXPECT_EQ(dll_open(invalid_utf8, DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");
#endif
}

TEST(dll, mode_flags)
{
    std::string dll_file = get_test_dll_path();

    dll_mode_t mode   = DLL_MODE_RTLD_NOW | DLL_MODE_RTLD_LOCAL;
    void      *handle = dll_open(dll_file.c_str(), mode);
    ASSERT_NE(handle, nullptr);
    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll, macros)
{
    EXPECT_FALSE(std::string(DLL_EXT).empty());
}

TEST(dll, unicode_utf8_path_loading)
{
    std::string original_dll = get_test_dll_path();

    fs::path unicode_dir = "./测试目录_Unicode_Path";
    fs::create_directories(unicode_dir);

    fs::path unicode_dll_path =
        unicode_dir
        / ("测试插件_" + std::string(DLL_PREFIX) + "dll_example" + DLL_EXT);

    std::error_code ec;
    fs::copy_file(original_dll,
                  unicode_dll_path,
                  fs::copy_options::overwrite_existing,
                  ec);
    ASSERT_FALSE(ec) << "Failed to prepare unicode test file: " << ec.message();

    void *handle =
        dll_open(unicode_dll_path.u8string().c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr)
        << "Failed to load DLL from Unicode path: " << dll_pop_error();

    auto hello = (hello_fn_t) dll_get(handle, "hello");
    ASSERT_NE(hello, nullptr);
    EXPECT_EQ(hello(), 1);

    EXPECT_EQ(dll_close(handle), 0);

    fs::remove_all(unicode_dir, ec);
}

TEST(dll, empty_string_inputs)
{
    EXPECT_EQ(dll_open("", DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    std::string dll_file = get_test_dll_path();
    void       *handle   = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr);

    EXPECT_EQ(dll_get(handle, ""), nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    EXPECT_EQ(dll_close(handle), 0);
}

TEST(dll, invalid_mode_flags)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), (dll_mode_t) 0xFFFFFFFF);
    EXPECT_EQ(handle, nullptr);
    EXPECT_STRNE(dll_pop_error(), "");

    void *handle2 = dll_open(dll_file.c_str(), (1 << 29));
    EXPECT_EQ(handle2, nullptr);
    EXPECT_STRNE(dll_pop_error(), "");
}

TEST(dll, close_lifetime_contract)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr);

    EXPECT_EQ(dll_close(handle), 0);
    EXPECT_NE(dll_close(NULL), 0);

    hj::dll_loader loader(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(loader.is_loaded());

    loader.close();
    EXPECT_FALSE(loader.is_loaded());
    EXPECT_EQ(loader.get(), nullptr);
    EXPECT_EQ(loader.symbol<hello_fn_t>("hello"), nullptr);
}