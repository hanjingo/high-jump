#include <gtest/gtest.h>
#include <hj/testing/crash.hpp>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

int run_crasher_subprocess(const std::string &crash_type,
                           const std::string &dump_dir)
{
    std::error_code ec;
    if(fs::exists(dump_dir))
    {
        fs::remove_all(dump_dir, ec);
    }
    fs::create_directories(dump_dir, ec);

#if defined(_WIN32)
    std::string cmd = "crasher.exe --type " + crash_type + " --dir " + dump_dir;
#else
    std::string cmd = "./crasher --type " + crash_type + " --dir " + dump_dir;
#endif
    return std::system(cmd.c_str());
}

bool check_dump_file_exists(const std::string &dump_dir)
{
    if(!fs::exists(dump_dir) || !fs::is_directory(dump_dir))
    {
        return false;
    }
    for(const auto &entry : fs::directory_iterator(dump_dir))
    {
        if(entry.is_regular_file() && entry.path().extension() == ".dmp")
        {
            if(entry.file_size() > 0)
            {
                return true;
            }
        }
    }
    return false;
}

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
}
#elif defined(__APPLE__)
static bool test_callback(const char *, const char *, void *, bool)
{
    crash_called = true;
    return true;
}
#else
static bool
test_callback(const google_breakpad::MinidumpDescriptor &, void *, bool)
{
    crash_called = true;
    return true;
}
#endif

TEST(crash, singleton)
{
    auto *h1 = hj::crash_handler::instance();
    auto *h2 = hj::crash_handler::instance();
    EXPECT_EQ(h1, h2);
}

TEST(crash, prevent_exception_filter)
{
    auto *handler = hj::crash_handler::instance();
#if defined(_WIN32)
    EXPECT_TRUE(handler->prevent_set_unhandled_exception_filter());
#else
    SUCCEED();
#endif
}

TEST(crash, init_handler)
{
    auto *handler = hj::crash_handler::instance();
    handler->init("./test_dumps_init", test_callback);
    SUCCEED();
}

TEST(crash, segfault_capture)
{
    std::string dump_dir = "./test_dumps_segfault";
    int         ret      = run_crasher_subprocess("segfault", dump_dir);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(check_dump_file_exists(dump_dir))
        << "Minidump file not generated for segfault!";
}

TEST(crash, div_by_zero_capture)
{
    std::string dump_dir = "./test_dumps_div";
    int         ret      = run_crasher_subprocess("divbyzero", dump_dir);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(check_dump_file_exists(dump_dir))
        << "Minidump file not generated for divbyzero!";
}

TEST(crash, abort_capture)
{
    std::string dump_dir = "./test_dumps_abort";
    int         ret      = run_crasher_subprocess("abort", dump_dir);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(check_dump_file_exists(dump_dir))
        << "Minidump file not generated for abort!";
}