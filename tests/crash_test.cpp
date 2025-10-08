#include <gtest/gtest.h>
#include <hj/testing/crash.hpp>
#include <string>

static bool crash_called = false;
#if defined(_WIN32)
static bool test_callback(const wchar_t *,
                          const wchar_t *,
                          void *,
                          EXCEPTION_POINTERS *,
                          MDRawAssertionInfo *,
                          bool)
{
    crash_called = true;
    return true;
};
#elif defined(__APPLE__)
static bool test_callback(const char *, const char *, void *, bool)
{
    crash_called = true;
    return true;
};
#else
static bool
test_callback(const google_breakpad::MinidumpDescriptor &, void *, bool)
{
    crash_called = true;
    return true;
};
#endif

TEST(crash, set_local_path)
{
    auto *handler = hj::crash_handler::instance();
    EXPECT_TRUE(handler->prevent_set_unhandled_exception_filter());
    handler->set_local_path("./test1");
}

TEST(crash, set_dump_callback)
{
    auto *handler = hj::crash_handler::instance();
    handler->set_dump_callback(test_callback);
    handler->set_local_path("./test2");
    EXPECT_FALSE(crash_called);
}

TEST(crash, singleton)
{
    auto *h1 = hj::crash_handler::instance();
    auto *h2 = hj::crash_handler::instance();
    EXPECT_EQ(h1, h2);
}

TEST(crash, set_local_path_multiple)
{
    auto *handler = hj::crash_handler::instance();
    handler->set_local_path("./test3");
    handler->set_local_path("./test4");
    SUCCEED();
}
