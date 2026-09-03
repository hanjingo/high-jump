#include <gtest/gtest.h>
#include <hj/os/dll.h>

typedef int (*hello)(void);
typedef int (*world)(void);

TEST(dll, dll_open_close)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    auto handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle != NULL);
    ASSERT_TRUE(dll_close(handle));
}

TEST(dll, repeated_open)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    auto handle_a = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle_a != NULL);

    auto handle_b = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle_b != NULL);

    auto fn1 = dll_get(handle_b, "hello");
    ASSERT_TRUE(fn1 != NULL);
    EXPECT_EQ(((hello) fn1)(), 1);

    ASSERT_TRUE(dll_close(handle_a));
    ASSERT_TRUE(dll_close(handle_b));
}

TEST(dll, close_twice)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    auto handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);

    ASSERT_TRUE(dll_close(handle));
    EXPECT_FALSE(dll_close(handle));
}

TEST(dll, dll_get_call)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    auto handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle != NULL);

    hello fn1 = (hello) dll_get(handle, "hello");
    ASSERT_TRUE(fn1 != NULL);
    ASSERT_EQ(fn1(), 1);

    world fn2 = (world) dll_get(handle, "world");
    ASSERT_TRUE(fn2 != NULL);
    ASSERT_EQ(fn2(), 2);

    ASSERT_TRUE(dll_close(handle));
}

TEST(dll, invalid_symbol_queries)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    auto handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle != NULL);

    auto invalid_fn = dll_get(handle, "this_symbol_does_not_exist");
    ASSERT_FALSE(invalid_fn != NULL);

    auto null_sym = dll_get(handle, NULL);
    ASSERT_FALSE(null_sym != NULL);

    void *empty_handle = nullptr;
    EXPECT_EQ(dll_get(empty_handle, "hello"), nullptr);
    ASSERT_TRUE(dll_close(handle));
}

TEST(dll, dll_ext)
{
    ASSERT_EQ(!std::string(DLL_EXT).empty(), true);
}

TEST(dll, test_rtld_noload_semantics)
{
#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif

    void *probe_handle = dll_open(dll_file.c_str(), DLL_MODE_RTLD_NOLOAD);
    ASSERT_FALSE(probe_handle != NULL);

    void *real_handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(real_handle != NULL);

    void *noload_handle = dll_open(dll_file.c_str(), DLL_MODE_RTLD_NOLOAD);
    ASSERT_TRUE(noload_handle != NULL);

    if(noload_handle)
        dll_close(noload_handle);

    dll_close(real_handle);
}

TEST(dll, invalid_inputs)
{
    ASSERT_FALSE(dll_open(NULL, DLL_MODE_DEFAULT) != NULL);
    ASSERT_FALSE(dll_open("./not_exist_library.so", DLL_MODE_DEFAULT) != NULL);

    ASSERT_FALSE(dll_get(NULL, "hello") != NULL);

#ifdef _WIN32
    auto dll_file = std::string("./dll_example").append(DLL_EXT);
#else
    auto dll_file = std::string("./libdll_example").append(DLL_EXT);
#endif
    void *handle = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle != NULL);
    ASSERT_FALSE(dll_get(handle, "non_existent_symbol") != NULL);
    ASSERT_FALSE(dll_get(handle, NULL) != NULL);

    EXPECT_FALSE(dll_close(NULL));
    EXPECT_TRUE(dll_close(handle));
}
