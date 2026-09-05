// sdk_layout_test.cpp
#include <gtest/gtest.h>
#include <hj/os/sdk.h>
#include <hj/os/dll.h>
#include <cstddef>
#include <stddef.h>
#include <assert.h>

void test_c_compatibility_compile_time(void)
{
    sdk_context_t ctx;
    ctx.user_data = NULL;
    ctx.callback  = NULL;

    (void) ctx;
}

void world_cb_v1(void *user_data)
{
    struct world_param_v1_t
    {
        SDK_ABI_HEADER

        const char *in;
        uint32_t    in_len;

        char    *out;
        uint32_t out_len;
    };

    std::cout << "world callback invoked!" << std::endl;
    auto *param = static_cast<world_param_v1_t *>(user_data);
    if(param != nullptr)
    {
        std::cout << "param.in: " << param->in << std::endl;
        std::cout << "param.in_len: " << param->in_len << std::endl;
        std::cout << "param.out: " << param->out << std::endl;
        std::cout << "param.out_len: " << param->out_len << std::endl;
    }

    ASSERT_STREQ(param->out, "Hello from sdk_v1!");
    ASSERT_EQ(param->out_len, 18);
}

void world_cb_v2(void *user_data)
{
    struct world_param_v2_t
    {
        SDK_ABI_HEADER

        const char *in;
        uint32_t    in_len;

        const char *memo;
        uint32_t    memo_len;

        char    *out;
        uint32_t out_len;
    };

    std::cout << "world callback invoked!" << std::endl;
    auto *param = static_cast<world_param_v2_t *>(user_data);
    if(param != nullptr)
    {
        std::cout << "param.in: " << param->in << std::endl;
        std::cout << "param.in_len: " << param->in_len << std::endl;
        std::cout << "param.memo: " << param->memo << std::endl;
        std::cout << "param.memo_len: " << param->memo_len << std::endl;
        std::cout << "param.out: " << param->out << std::endl;
        std::cout << "param.out_len: " << param->out_len << std::endl;
    }

    ASSERT_STREQ(param->out, "Hello from sdk_v2!");
    ASSERT_EQ(param->out_len, 18);
}

TEST(sdk, call_sdk_v1)
{
    struct hello_param_v1_t
    {
        SDK_ABI_HEADER

        int32_t num;
    } hello_param;

    struct world_param_v1_t
    {
        SDK_ABI_HEADER

        const char *in;
        uint32_t    in_len;

        char    *out;
        uint32_t out_len;
    } world_param;

    std::string dll_file = std::string("./sdk_v1") + DLL_EXT;
    auto        handle   = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    if(!handle)
    {
        GTEST_SKIP() << "Failed to open sdk_v1 DLL: " << dll_file;
        return;
    }

    SDK_CONTEXT(ctx);

    // call hello
    SDK_INIT_ABI_HEADER(&hello_param, hello_param_v1_t, 1);
    hello_param.num = 123;
    ctx.user_data   = &hello_param;
    ctx.callback    = NULL;
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");
    hello_func(&ctx);

    // call world
    SDK_INIT_ABI_HEADER(&world_param, world_param_v1_t, 1);
    world_param.in      = "Hello";
    world_param.in_len  = strlen(world_param.in);
    world_param.out     = new char[256];
    world_param.out_len = 256;

    ctx.user_data = &world_param;
    ctx.callback  = world_cb_v1;

    auto world_func = (sdk_api_t) dll_get(handle, "world");
    world_func(&ctx);
    delete[] world_param.out;
}

TEST(sdk, call_sdk_v2)
{
    struct hello_param_v2_t
    {
        SDK_ABI_HEADER

        int32_t num;

        const char *tag;
        uint32_t    tag_len;
    } hello_param;

    struct world_param_v2_t
    {
        SDK_ABI_HEADER

        const char *in;
        uint32_t    in_len;

        const char *memo;
        uint32_t    memo_len;

        char    *out;
        uint32_t out_len;
    } world_param;

    std::string dll_file = std::string("./sdk_v2") + DLL_EXT;
    auto        handle   = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    if(!handle)
    {
        GTEST_SKIP() << "Failed to open sdk_v1 DLL: " << dll_file;
        return;
    }

    SDK_CONTEXT(ctx);

    // call hello
    SDK_INIT_ABI_HEADER(&hello_param, hello_param_v2_t, 2);
    hello_param.num     = 123;
    hello_param.tag     = "hello param tag v2";
    hello_param.tag_len = strlen(hello_param.tag);
    ctx.user_data       = &hello_param;
    ctx.callback        = NULL;
    auto hello_func     = (sdk_api_t) dll_get(handle, "hello");
    hello_func(&ctx);

    // call world
    SDK_INIT_ABI_HEADER(&world_param, world_param_v2_t, 2);
    world_param.in       = "Hello";
    world_param.in_len   = strlen(world_param.in);
    world_param.memo     = "world param tag v2";
    world_param.memo_len = strlen(world_param.memo);
    world_param.out      = new char[256];
    world_param.out_len  = 256;

    ctx.user_data = &world_param;
    ctx.callback  = world_cb_v2;

    auto world_func = (sdk_api_t) dll_get(handle, "world");
    world_func(&ctx);
    delete[] world_param.out;
}

TEST(sdk, static_layout_and_alignment)
{
    static_assert(sizeof(sdk_context_t) == 32,
                  "ABI Error: sdk_context_t size mismatch!");

    static_assert(alignof(sdk_context_t) == 8,
                  "ABI Error: sdk_context_t alignment mismatch!");

    static_assert(offsetof(sdk_context_t, abi_magic) == 0,
                  "ABI Error: abi_magic offset moved!");
    static_assert(offsetof(sdk_context_t, abi_size) == 4,
                  "ABI Error: abi_size offset moved!");
    static_assert(offsetof(sdk_context_t, abi_version) == 8,
                  "ABI Error: abi_version offset moved!");

    static_assert(offsetof(sdk_context_t, user_data) == 16,
                  "ABI Error: user_data alignment padding error!");
    static_assert(offsetof(sdk_context_t, callback) == 24,
                  "ABI Error: callback offset moved!");
}

TEST(sdk, legacy_v1_client_calling_v2_sdk)
{
    struct legacy_hello_param_v1
    {
        SDK_ABI_HEADER

        int32_t num;
    } legacy_param;

    SDK_INIT_ABI_HEADER(&legacy_param, legacy_hello_param_v1, 1);
    legacy_param.num = 8888;

    SDK_CONTEXT(ctx);
    ctx.user_data = &legacy_param;

    // load sdk_v2!!!
    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);

    auto hello_func = (sdk_api_t) dll_get(handle, "hello");
    ASSERT_TRUE(hello_func);

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, uninitialized_header_protection)
{
    struct uninitialized_param
    {
        const char *raw_str;
        int         raw_int;
    } bad_param;

    bad_param.raw_str = "Illegal raw memory";
    bad_param.raw_int = 1234;

    SDK_CONTEXT(ctx);
    ctx.user_data = &bad_param;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);

    auto hello_func = (sdk_api_t) dll_get(handle, "hello");
    ASSERT_TRUE(hello_func);

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, null_pointer_handling)
{
    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");
    ASSERT_TRUE(hello_func);

    EXPECT_NO_FATAL_FAILURE(hello_func(NULL));
}

TEST(sdk, corrupted_magic)
{
    SDK_CONTEXT(ctx);
    ctx.abi_magic = 0xDEADBEEF;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");
    ASSERT_TRUE(hello_func);

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, malformed_zero_size)
{
    struct hello_param_v2_t
    {
        SDK_ABI_HEADER
        int32_t num;
    } param;

    SDK_INIT_ABI_HEADER(&param, hello_param_v2_t, 2);
    param.abi_size = 0;

    SDK_CONTEXT(ctx);
    ctx.user_data = &param;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, truncated_size_boundary)
{
    struct hello_param_v2_t
    {
        SDK_ABI_HEADER
        int32_t     num;
        const char *tag;
        uint32_t    tag_len;
    } param;

    SDK_INIT_ABI_HEADER(&param, hello_param_v2_t, 2);
    param.abi_size = offsetof(hello_param_v2_t, num) + sizeof(param.num);

    param.tag     = "This string should NOT be accessed";
    param.tag_len = (uint32_t) strlen(param.tag);

    SDK_CONTEXT(ctx);
    ctx.user_data = &param;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, wrong_future_version_handling)
{
    struct hello_param_v2_t
    {
        SDK_ABI_HEADER
        int32_t num;
    } param;

    SDK_INIT_ABI_HEADER(&param, hello_param_v2_t, 999U);
    param.num = 777;

    SDK_CONTEXT(ctx);
    ctx.user_data = &param;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}

TEST(sdk, misaligned_pointer_handling)
{
    uint8_t   buffer[64];
    uintptr_t unaligned_addr = (uintptr_t) buffer;
    if(unaligned_addr % 8 == 0)
    {
        unaligned_addr += 4;
    }

    auto *misaligned_ctx = reinterpret_cast<sdk_context_t *>(unaligned_addr);
    misaligned_ctx->abi_magic   = SDK_ABI_MAGIC;
    misaligned_ctx->abi_size    = sizeof(sdk_context_t);
    misaligned_ctx->abi_version = SDK_CONTEXT_ABI_VERSION_1;
    misaligned_ctx->user_data   = NULL;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");

    EXPECT_NO_FATAL_FAILURE(hello_func(misaligned_ctx));
}

TEST(sdk, oversized_abi_size_attack_protection)
{
    struct hello_param_v2_t
    {
        SDK_ABI_HEADER
        int32_t     num;
        const char *tag;
        uint32_t    tag_len;
    } param;

    SDK_INIT_ABI_HEADER(&param, hello_param_v2_t, 2);
    param.abi_size = 0xFFFFFFFF;

    SDK_CONTEXT(ctx);
    ctx.user_data = &param;

    auto handle = dll_open("./sdk_v2" DLL_EXT, DLL_MODE_DEFAULT);
    ASSERT_TRUE(handle);
    auto hello_func = (sdk_api_t) dll_get(handle, "hello");

    EXPECT_NO_FATAL_FAILURE(hello_func(&ctx));
}