#include <gtest/gtest.h>
#include <hj/os/dll.h>

#include <string>
#include <thread>
#include <vector>

typedef int (*hello_fn_t)(void);
typedef int (*world_fn_t)(void);

static std::string get_test_dll_path()
{
    return std::string("./") + DLL_PREFIX + "dll_example" + DLL_EXT;
}

TEST(dll_test, open_and_close)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr) << "Failed to open: " << dll_error();
    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll_test, repeated_open)
{
    std::string dll_file = get_test_dll_path();

    void *handle_a = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle_a, nullptr) << "Error: " << dll_error();

    void *handle_b = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle_b, nullptr) << "Error: " << dll_error();

    auto fn1 = (hello_fn_t) dll_get(handle_b, "hello");
    ASSERT_NE(fn1, nullptr) << "Error: " << dll_error();
    EXPECT_EQ(fn1(), 1);

    EXPECT_TRUE(dll_close(handle_a) == 0);
    EXPECT_TRUE(dll_close(handle_b) == 0);
}

TEST(dll_test, get_and_call_symbol)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr) << "Error: " << dll_error();

    hello_fn_t fn_hello = (hello_fn_t) dll_get(handle, "hello");
    ASSERT_NE(fn_hello, nullptr) << "Error: " << dll_error();
    EXPECT_EQ(fn_hello(), 1);

    world_fn_t fn_world = (world_fn_t) dll_get(handle, "world");
    ASSERT_NE(fn_world, nullptr) << "Error: " << dll_error();
    EXPECT_EQ(fn_world(), 2);

    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll_test, invalid_symbol_queries)
{
    std::string dll_file = get_test_dll_path();

    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(handle, nullptr);

    void *invalid_fn = dll_get(handle, "this_symbol_does_not_exist");
    EXPECT_EQ(invalid_fn, nullptr);
    EXPECT_STRNE(dll_error(), "");

    void *null_sym = dll_get(handle, NULL);
    EXPECT_EQ(null_sym, nullptr);
    EXPECT_STRNE(dll_error(), "");

    EXPECT_EQ(dll_get(NULL, "hello"), nullptr);
    EXPECT_STRNE(dll_error(), "");

    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll_test, rtld_noload_semantics)
{
    void *probe_not_loaded =
        dll_open("./not_exist_module_for_test.dll", DLL_MODE_RTLD_NOLOAD);
    EXPECT_EQ(probe_not_loaded, nullptr);
    EXPECT_STRNE(dll_error(), "");

    std::string dll_file = get_test_dll_path();

    void *real_handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_NE(real_handle, nullptr) << "Error: " << dll_error();

    void *noload_handle = dll_open(dll_file.c_str(), DLL_MODE_RTLD_NOLOAD);
    EXPECT_NE(noload_handle, nullptr);

    if(noload_handle)
    {
        dll_close(noload_handle);
    }

    EXPECT_TRUE(dll_close(real_handle) == 0);
}

TEST(dll_test, invalid_inputs)
{
    EXPECT_EQ(dll_open(NULL, DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_error(), "");

    EXPECT_EQ(dll_open("./not_exist_library.so", DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_error(), "");

    EXPECT_FALSE(dll_close(NULL) == 0);
    EXPECT_STRNE(dll_error(), "");
}

TEST(dll_test, error_handling_and_clearing)
{
    dll_clear_error();
    EXPECT_STREQ(dll_error(), "");

    dll_open(NULL, DLL_MODE_DEFAULT);
    EXPECT_STRNE(dll_error(), "");

    dll_clear_error();
    EXPECT_STREQ(dll_error(), "");
}

TEST(dll_test, multithreaded_error_isolation)
{
    dll_clear_error();

    std::thread t([]() {
        dll_open(NULL, DLL_MODE_DEFAULT);
        EXPECT_STRNE(dll_error(), "");
    });

    t.join();

    EXPECT_STREQ(dll_error(), "");
}

TEST(dll_test, long_path_and_utf8)
{
    std::string long_path = "./";
    for(int i = 0; i < 30; ++i)
    {
        long_path += "very_long_path_component_";
    }
    long_path += DLL_EXT;

    EXPECT_EQ(dll_open(long_path.c_str(), DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_error(), "");

#if defined(_WIN32)
    const char invalid_utf8[] = {'\xC0', '\xAF', '\0'};
    EXPECT_EQ(dll_open(invalid_utf8, DLL_MODE_DEFAULT), nullptr);
    EXPECT_STRNE(dll_error(), "");
#endif
}

TEST(dll_test, mode_flags)
{
    std::string dll_file = get_test_dll_path();

    dll_mode_t mode   = DLL_MODE_RTLD_NOW | DLL_MODE_RTLD_LOCAL;
    void      *handle = dll_open(dll_file.c_str(), mode);
    ASSERT_NE(handle, nullptr);
    EXPECT_TRUE(dll_close(handle) == 0);
}

TEST(dll_test, macros)
{
    EXPECT_FALSE(std::string(DLL_EXT).empty());
}