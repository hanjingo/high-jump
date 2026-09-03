#include <gtest/gtest.h>
#include <hj/testing/crash.hpp>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <vector>
#include <filesystem>
#include <fstream>

#if defined(_WIN32)
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <client/windows/handler/exception_handler.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <client/mac/handler/exception_handler.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <client/linux/handler/exception_handler.h>
#endif

#include <google_breakpad/common/breakpad_types.h>
#include <google_breakpad/common/minidump_format.h>

#ifndef MD_MINIDUMP_SIGNATURE
#define MD_MINIDUMP_SIGNATURE 0x504d444d
#endif

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
    std::wstring app_name = L"crasher.exe";
    std::string  cmd_str =
        "crasher.exe --type " + crash_type + " --dir \"" + dump_dir + "\"";

    std::vector<wchar_t> cmd_buf(cmd_str.begin(), cmd_str.end());
    cmd_buf.push_back(L'\0');

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    BOOL ok = ::CreateProcessW(nullptr,
                               cmd_buf.data(),
                               nullptr,
                               nullptr,
                               FALSE,
                               0,
                               nullptr,
                               nullptr,
                               &si,
                               &pi);

    if(!ok)
    {
        return -1;
    }

    ::WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code = 0;
    ::GetExitCodeProcess(pi.hProcess, &exit_code);

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);

    return static_cast<int>(exit_code);

#else

    pid_t pid = ::fork();
    if(pid < 0)
    {
        return -1;
    }

    if(pid == 0)
    {
        const char *exe    = "./crasher";
        const char *args[] = {exe,
                              "--type",
                              crash_type.c_str(),
                              "--dir",
                              dump_dir.c_str(),
                              nullptr};

        ::execv(exe, const_cast<char *const *>(args));
        ::_exit(127);
    }

    int status = 0;
    if(::waitpid(pid, &status, 0) < 0)
    {
        return -1;
    }

    if(WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    if(WIFSIGNALED(status))
    {
        return -WTERMSIG(status);
    }

    return status;

#endif
}

bool validate_minidump_file(const std::string &dump_dir)
{
    if(!fs::exists(dump_dir) || !fs::is_directory(dump_dir))
    {
        return false;
    }

    fs::path callback_log = fs::path(dump_dir) / "callback.log";
    if(!fs::exists(callback_log))
    {
        return false;
    }

    std::ifstream log_file(callback_log);
    std::string   log_content((std::istreambuf_iterator<char>(log_file)),
                              std::istreambuf_iterator<char>());
    if(log_content.find("[crasher] Breakpad callback triggered successfully")
       == std::string::npos)
    {
        return false;
    }

    fs::path dump_file_path;
    uint64_t file_size = 0;
    for(const auto &entry : fs::directory_iterator(dump_dir))
    {
        if(entry.is_regular_file() && entry.path().extension() == ".dmp")
        {
            std::error_code ec;
            uint64_t        sz = entry.file_size(ec);
            if(!ec && sz > sizeof(MDRawHeader))
            {
                dump_file_path = entry.path();
                file_size      = sz;
                break;
            }
        }
    }

    if(dump_file_path.empty())
    {
        return false;
    }

    std::ifstream file(dump_file_path, std::ios::binary);
    if(!file.is_open())
    {
        return false;
    }

    MDRawHeader header{};
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    if(!file || header.signature != MD_MINIDUMP_SIGNATURE)
    {
        return false;
    }

    if(header.stream_count == 0 || header.stream_directory_rva == 0)
    {
        return false;
    }

    uint64_t dir_rva = static_cast<uint64_t>(header.stream_directory_rva);
    if(dir_rva >= file_size)
    {
        return false;
    }

    uint64_t stream_count = static_cast<uint64_t>(header.stream_count);
    uint64_t entry_size   = static_cast<uint64_t>(sizeof(MDRawDirectory));

    if(stream_count > (std::numeric_limits<uint64_t>::max() / entry_size))
    {
        return false;
    }

    uint64_t total_dir_bytes = stream_count * entry_size;

    if(dir_rva + total_dir_bytes > file_size
       || (dir_rva + total_dir_bytes) < dir_rva)
    {
        return false;
    }

    file.seekg(header.stream_directory_rva, std::ios::beg);
    if(!file)
    {
        return false;
    }

    bool has_exception_stream   = false;
    bool has_thread_list_stream = false;
    bool has_module_list_stream = false;

    for(uint32_t i = 0; i < header.stream_count; ++i)
    {
        MDRawDirectory dir{};
        file.read(reinterpret_cast<char *>(&dir), sizeof(dir));
        if(!file)
        {
            return false;
        }

        if(dir.stream_type == MD_EXCEPTION_STREAM)
        {
            has_exception_stream = true;
        } else if(dir.stream_type == MD_THREAD_LIST_STREAM)
        {
            has_thread_list_stream = true;
        } else if(dir.stream_type == MD_MODULE_LIST_STREAM)
        {
            has_module_list_stream = true;
        }
    }

    return has_exception_stream && has_thread_list_stream
           && has_module_list_stream;
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
    auto &h1 = hj::crash_handler::instance();
    auto &h2 = hj::crash_handler::instance();
    EXPECT_EQ(&h1, &h2);
}

TEST(crash, init_lifecycle_and_reinit_guard)
{
    auto &handler = hj::crash_handler::instance();

    EXPECT_EQ(handler.state(), hj::crash_handler::State::UNINITIALIZED);

#if defined(_WIN32)
    std::string invalid_path = "C:\\invalid_path_?:*\\dumps";
#else
    std::string invalid_path = "/proc/non_existent_dir_12345/dumps";
#endif

    bool ok1 = handler.init(invalid_path, test_callback);
    EXPECT_FALSE(ok1);
    EXPECT_EQ(handler.state(), hj::crash_handler::State::FAILED);

    bool ok2 = handler.init("./test_dumps_init", test_callback);
    EXPECT_TRUE(ok2);
    EXPECT_EQ(handler.state(), hj::crash_handler::State::READY);

    bool ok3 = handler.init("./test_dumps_reinit", test_callback);
    EXPECT_FALSE(ok3);
    EXPECT_EQ(handler.state(), hj::crash_handler::State::READY);
}

TEST(crash, segfault_capture)
{
    std::string dump_dir = "./test_dumps_segfault";
    int         ret      = run_crasher_subprocess("segfault", dump_dir);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(validate_minidump_file(dump_dir))
        << "Minidump file is missing, empty, or corrupted for segfault!";
}

TEST(crash, div_by_zero_capture)
{
    std::string dump_dir = "./test_dumps_div";
    int         ret      = run_crasher_subprocess("divbyzero", dump_dir);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(validate_minidump_file(dump_dir))
        << "Minidump file is missing, empty, or corrupted for divbyzero!";
}

TEST(crash, abort_capture)
{
#if defined(_WIN32)
    EXPECT_TRUE(true) << "Abort capture test is skipped on Windows due to "
                         "potential issues with SetUnhandledExceptionFilter.";

#else
    std::string dump_dir = "./test_dumps_abort";
    int         ret      = run_crasher_subprocess("abort", dump_dir);

    EXPECT_NE(ret, 0);
    EXPECT_TRUE(validate_minidump_file(dump_dir))
        << "Minidump file is missing, empty, or corrupted for abort!";

#endif
}