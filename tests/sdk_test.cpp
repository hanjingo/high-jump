// sdk_layout_test.cpp
#include <gtest/gtest.h>
#include <hj/os/sdk.h>
#include <hj/os/dll.h>
#include <cstddef>
#include <stddef.h>
#include <assert.h>

#include "./sdk_v1/sdk_v1.h"
#include "./sdk_v2/sdk_v2.h"

void test_c_compatibility_compile_time(void)
{
    sdk_context_t ctx;
    ctx.size      = (uint32_t) sizeof(sdk_context_t);
    ctx.version   = SDK_CONTEXT_VERSION_1;
    ctx.user_data = NULL;
    ctx.callback  = NULL;

    (void) ctx;
}

TEST(sdk, layout_and_alignment)
{
    static_assert(sizeof(sdk_context_t) == 24,
                  "ABI Error: sdk_context_t size mismatch!");

    static_assert(alignof(sdk_context_t) == 8,
                  "ABI Error: sdk_context_t alignment mismatch!");

    static_assert(offsetof(sdk_context_t, size) == 0,
                  "ABI Error: size offset moved!");
    static_assert(offsetof(sdk_context_t, version) == 4,
                  "ABI Error: version offset moved!");
    static_assert(offsetof(sdk_context_t, user_data) == 8,
                  "ABI Error: user_data offset moved!");
    static_assert(offsetof(sdk_context_t, callback) == 16,
                  "ABI Error: callback offset moved!");
}

// TEST(sdk, callback_invocation_and_user_data)
// {
//     struct TestFixture
//     {
//         bool invoked = false;
//         int  value   = 0;
//     } fixture;

//     sdk_context_t ctx{};
//     ctx.size      = sizeof(sdk_context_t);
//     ctx.version   = SDK_CONTEXT_VERSION_1;
//     ctx.user_data = &fixture; // Borrowed pointer

//     ctx.callback = [](void *user_data) {
//         auto *f    = static_cast<TestFixture *>(user_data);
//         f->invoked = true;
//         f->value   = 42;
//     };

//     EXPECT_EQ(hj_sdk_run(&ctx), HJ_SUCCESS);

//     EXPECT_TRUE(fixture.invoked);
//     EXPECT_EQ(fixture.value, 42);
// }

// TEST(sdk, backward_compatibility_simulation)
// {
//     struct sdk_context_v0
//     {
//         uint32_t       size;
//         uint32_t       version;
//         void          *user_data;
//         sdk_callback_t callback;
//     } legacy_ctx;

//     legacy_ctx.size      = sizeof(sdk_context_v0);
//     legacy_ctx.version   = 0U;
//     legacy_ctx.user_data = nullptr;
//     legacy_ctx.callback  = nullptr;

//     auto *current_ctx = reinterpret_cast<sdk_context_t *>(&legacy_ctx);

//     EXPECT_NO_FATAL_FAILURE(hj_sdk_run(current_ctx));
// }

void world_cb(void *user_data)
{
    std::cout << "world callback invoked!" << std::endl;
    world_param_t *param = (world_param_t *) user_data;
    if(param != nullptr)
    {
        std::cout << "param.in: " << param->in << std::endl;
        std::cout << "param.in_len: " << param->in_len << std::endl;
        std::cout << "param.out: " << param->out << std::endl;
        std::cout << "param.out_len: " << param->out_len << std::endl;
    }
}

TEST(sdk, call_sdk_v1)
{
    std::string dll_file = std::string("./sdk_v1") + DLL_EXT;
    auto        handle   = dll_open(dll_file.c_str(), DLL_MODE_DEFAULT);
    if(!handle)
    {
        GTEST_SKIP() << "Failed to open sdk_v1 DLL: " << dll_file;
        return;
    }

    hello_param_t *hello_param = new hello_param_t();
    hello_param->num           = 123;
    sdk_context_t *ctx         = new sdk_context_t();
    ctx->size                  = (uint32_t) sizeof(sdk_context_t);
    ctx->version               = 1;
    ctx->user_data             = hello_param; // Example user data
    ctx->callback              = NULL;
    auto hello_func            = (sdk_api_t) dll_get(handle, "hello");
    hello_func(ctx);
    delete hello_param;

    world_param_t param;
    param.in        = "Hello";
    param.in_len    = strlen(param.in);
    param.out       = new char[256];
    param.out_len   = 256;
    ctx->size       = (uint32_t) sizeof(sdk_context_t);
    ctx->version    = 1;
    ctx->user_data  = &param;
    ctx->callback   = world_cb;
    auto world_func = (sdk_api_t) dll_get(handle, "world");
    world_func(ctx);
    delete[] param.out;

    delete ctx;
}