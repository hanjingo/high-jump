#ifndef CRASH_HPP
#define CRASH_HPP

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <BaseTsd.h>
#include <signal.h>
#include <client/windows/handler/exception_handler.h>

typedef SSIZE_T ssize_t;

#elif defined(__APPLE__)

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <client/mac/handler/exception_handler.h>

#elif defined(__linux__)

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <client/linux/handler/exception_handler.h>

#endif

namespace hj
{

#if defined(_WIN32)

using dump_callback_t = bool (*)(const wchar_t *,
                                 const wchar_t *,
                                 void *,
                                 EXCEPTION_POINTERS *,
                                 MDRawAssertionInfo *,
                                 bool);

static bool default_dump_callback(const wchar_t      *dump_dir,
                                  const wchar_t      *minidump_id,
                                  void               *context,
                                  EXCEPTION_POINTERS *exinfo,
                                  MDRawAssertionInfo *assertion,
                                  bool                succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    (void) exinfo;
    (void) assertion;
    return succeeded;
}

#elif defined(__APPLE__)

using dump_callback_t = bool (*)(const char *, const char *, void *, bool);

static bool default_dump_callback(const char *dump_dir,
                                  const char *minidump_id,
                                  void       *context,
                                  bool        succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    return succeeded;
}

#else

using dump_callback_t = bool (*)(const google_breakpad::MinidumpDescriptor &,
                                 void *,
                                 bool);

static bool
default_dump_callback(const google_breakpad::MinidumpDescriptor &descriptor,
                      void                                      *context,
                      bool                                       succeeded)
{
    (void) descriptor;
    (void) context;
    return succeeded;
}

#endif

/*
 * These helpers are strictly Async-Signal-Safe:
 * No heap allocations, no locks, no std::string/std::filesystem.
 */
static inline size_t crash_safe_u64tostr(uint64_t val, char *buf)
{
    char   tmp[24];
    size_t i = 0;

    do
    {
        tmp[i++] = static_cast<char>('0' + (val % 10));
        val /= 10;
    } while(val > 0);

    for(size_t j = 0; j < i; ++j)
        buf[j] = tmp[i - 1 - j];

    buf[i] = '\0';
    return i;
}

#if defined(_WIN32)

static inline bool
crash_safe_write_all(HANDLE hFile, const char *buf, size_t count)
{
    if(hFile == INVALID_HANDLE_VALUE || !buf)
        return false;

    size_t total_written = 0;

    while(total_written < count)
    {
        size_t bytes_to_write = count - total_written;

        DWORD chunk_size = bytes_to_write > static_cast<size_t>(0xFFFFFFFFu)
                               ? 0xFFFFFFFFu
                               : static_cast<DWORD>(bytes_to_write);

        DWORD written = 0;

        if(!::WriteFile(hFile,
                        buf + total_written,
                        chunk_size,
                        &written,
                        nullptr))
        {
            return false;
        }

        if(written == 0)
            return false;

        total_written += static_cast<size_t>(written);
    }

    return true;
}

#else

static inline bool crash_safe_write_all(int fd, const char *buf, size_t count)
{
    if(fd < 0 || !buf)
        return false;

    size_t total_written = 0;

    while(total_written < count)
    {
        ssize_t written =
            ::write(fd, buf + total_written, count - total_written);

        if(written < 0)
        {
            if(errno == EINTR)
                continue;

            return false;
        }

        if(written == 0)
            return false;

        total_written += static_cast<size_t>(written);
    }

    return true;
}

#endif

/**
 * @brief Strictly Async-Signal-Safe crash logger.
 * Pure stack buffer operations, zero heap allocations, no std::string/filesystem.
 */
static inline void crash_print(const char *content,
                               const char *path = "crash.log")
{
    if(!content || !path)
        return;

    size_t content_len = 0;
    while(content[content_len] != '\0')
        ++content_len;

#if defined(_WIN32)
    wchar_t wpath[1024]{};
    int     wlen = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 1024);
    if(wlen <= 0)
        return;

    HANDLE hFile = ::CreateFileW(wpath,
                                 FILE_APPEND_DATA,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 nullptr,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 nullptr);

    if(hFile == INVALID_HANDLE_VALUE)
        return;

    SYSTEMTIME st{};
    ::GetLocalTime(&st);

    char timeBuf[64]{};
    int  timeLen = ::wsprintfA(timeBuf,
                               "%04d-%02d-%02d %02d:%02d:%02d : ",
                               st.wYear,
                               st.wMonth,
                               st.wDay,
                               st.wHour,
                               st.wMinute,
                               st.wSecond);

    if(timeLen > 0)
    {
        crash_safe_write_all(hFile, timeBuf, static_cast<size_t>(timeLen));
    }

    if(content_len > 0)
        crash_safe_write_all(hFile, content, content_len);

    crash_safe_write_all(hFile, "\r\n", 2);

    ::CloseHandle(hFile);

#else

    int fd = ::open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd < 0)
        return;

    struct timespec ts{};
    if(::clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        char   timeBuf[64]{};
        size_t len = 0;

        timeBuf[len++] = '[';
        len += crash_safe_u64tostr(static_cast<uint64_t>(ts.tv_sec),
                                   timeBuf + len);
        timeBuf[len++] = '.';
        len += crash_safe_u64tostr(static_cast<uint64_t>(ts.tv_nsec),
                                   timeBuf + len);
        timeBuf[len++] = ']';
        timeBuf[len++] = ' ';

        crash_safe_write_all(fd, timeBuf, len);
    }

    if(content_len > 0)
        crash_safe_write_all(fd, content, content_len);

    crash_safe_write_all(fd, "\n", 1);

    ::close(fd);

#endif
}

class crash_handler
{
  public:
    enum class State
    {
        UNINITIALIZED,
        READY,
        FAILED
    };

  public:
    static crash_handler &instance()
    {
        static crash_handler inst;
        return inst;
    }

    crash_handler(const crash_handler &)            = delete;
    crash_handler &operator=(const crash_handler &) = delete;
    crash_handler(crash_handler &&)                 = delete;
    crash_handler &operator=(crash_handler &&)      = delete;

    bool init(const std::string &abs_path, dump_callback_t cb = nullptr)
    {
        if(abs_path.empty())
            return false;

        std::lock_guard<std::mutex> lock(_init_mutex);
        if(_state.load(std::memory_order_relaxed) == State::READY)
            return false;

        _handler.reset();

        dump_callback_t       final_cb = cb ? cb : default_dump_callback;
        std::filesystem::path dir_path(abs_path);
        std::error_code       ec;
        std::filesystem::create_directories(dir_path, ec);
        if(ec || !std::filesystem::is_directory(dir_path, ec))
        {
            _state.store(State::FAILED, std::memory_order_release);
            return false;
        }

        try
        {
#if defined(_WIN32)
            _wabs_path = dir_path.wstring();
            _handler   = std::make_unique<google_breakpad::ExceptionHandler>(
                _wabs_path,
                nullptr, // FilterCallback
                final_cb,
                nullptr, // callback context
                google_breakpad::ExceptionHandler::HANDLER_ALL);

#elif defined(__APPLE__)
            _abs_path = dir_path.string();
            _handler  = std::make_unique<google_breakpad::ExceptionHandler>(
                _abs_path,
                nullptr, // FilterCallback
                final_cb,
                nullptr, // callback context
                true,
                nullptr);

#else
            _abs_path = dir_path.string();
            _handler  = std::make_unique<google_breakpad::ExceptionHandler>(
                google_breakpad::MinidumpDescriptor(_abs_path),
                nullptr, // FilterCallback
                final_cb,
                nullptr, // callback context
                true,
                -1);

#endif
        }
        catch(...)
        {
            _handler.reset();

            _state.store(State::FAILED, std::memory_order_release);
            return false;
        }

        if(!_handler)
        {
            _state.store(State::FAILED, std::memory_order_release);
            return false;
        }

        _state.store(State::READY, std::memory_order_release);
        return true;
    }

    bool init(const char *abs_path, dump_callback_t cb = nullptr)
    {
        if(!abs_path)
            return false;

        return init(std::string(abs_path), cb);
    }

    State state() const noexcept
    {
        return _state.load(std::memory_order_acquire);
    }

    bool is_ready() const noexcept
    {
        return _state.load(std::memory_order_acquire) == State::READY;
    }

    bool write_minidump()
    {
        if(_state.load(std::memory_order_acquire) != State::READY)
            return false;

        return _handler ? _handler->WriteMinidump() : false;
    }

#if defined(_WIN32)
    static LPTOP_LEVEL_EXCEPTION_FILTER
        WINAPI temp_set_unhandled_exception_filter(
            LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
    {
        (void) lpTopLevelExceptionFilter;
        return NULL;
    }
#endif

    [[deprecated(
        "Directly patching system DLL code (SetUnhandledExceptionFilter) is "
        "discouraged due to modern Windows security mechanisms (CFG/CET) and "
        "maintainability. Use standard Breakpad initialization instead.")]]
    bool prevent_set_unhandled_exception_filter()
    {
#if defined(_WIN32)
        HMODULE hKernel32 = LoadLibraryW(L"kernel32.dll");
        if(hKernel32 == NULL)
            return false;

        auto scope_guard = [&hKernel32]() {
            if(hKernel32)
            {
                ::FreeLibrary(hKernel32);
                hKernel32 = NULL;
            }
        };
        struct library_guard
        {
            HMODULE h;
            ~library_guard()
            {
                if(h)
                    ::FreeLibrary(h);
            }
        } lib_guard{hKernel32};

        void *pOrgEntry =
            (void *) (::GetProcAddress(hKernel32,
                                       "SetUnhandledExceptionFilter"));
        if(pOrgEntry == NULL)
            return false;

        SIZE_T        patchSize      = 0;
        unsigned char patchBytes[16] = {0};
        void         *pNewFunc =
            (void *) (&crash_handler::temp_set_unhandled_exception_filter);

#if defined(_WIN64)
        // MOV RAX, pNewFunc  (48 B8 [8-byte address])
        // JMP RAX            (FF E0)
        patchSize              = 12;
        patchBytes[0]          = 0x48;
        patchBytes[1]          = 0xB8;
        std::uint64_t funcAddr = reinterpret_cast<std::uint64_t>(pNewFunc);
        std::memcpy(&patchBytes[2], &funcAddr, sizeof(funcAddr));
        patchBytes[10] = 0xFF;
        patchBytes[11] = 0xE0;
#else
        // JMP rel32          (E9 [4-byte relative offset])
        patchSize                = 5;
        patchBytes[0]            = 0xE9;
        ULONG_PTR dwOrgEntryAddr = reinterpret_cast<ULONG_PTR>(pOrgEntry);
        ULONG_PTR dwNewEntryAddr = reinterpret_cast<ULONG_PTR>(pNewFunc);
        INT64     qwRelativeAddr = static_cast<INT64>(dwNewEntryAddr)
                                   - (static_cast<INT64>(dwOrgEntryAddr) + 5);
        if(qwRelativeAddr < INT32_MIN || qwRelativeAddr > INT32_MAX)
            return false;

        LONG dwRelativeAddr = static_cast<LONG>(qwRelativeAddr);
        std::memcpy(&patchBytes[1], &dwRelativeAddr, sizeof(LONG));
#endif
        DWORD dwOldFlag = 0, dwTempFlag = 0;
        if(!::VirtualProtect(pOrgEntry,
                             patchSize,
                             PAGE_EXECUTE_READWRITE,
                             &dwOldFlag))
            return false;

        _pOrgEntry = pOrgEntry;
        _patchSize = patchSize;
        std::memcpy(_origBytes, pOrgEntry, patchSize);
        std::memcpy(pOrgEntry, patchBytes, patchSize);
        ::FlushInstructionCache(GetCurrentProcess(), pOrgEntry, patchSize);
        ::VirtualProtect(pOrgEntry, patchSize, dwOldFlag, &dwTempFlag);
        _is_patched = true;
        return true;
#else
        return true;
#endif
    }

  private:
#if defined(_WIN32)
    void         *_pOrgEntry;
    SIZE_T        _patchSize;
    unsigned char _origBytes[16];
    bool          _is_patched;
#endif

  private:
    crash_handler()  = default;
    ~crash_handler() = default;

  private:
    std::mutex         _init_mutex;
    std::atomic<State> _state{State::UNINITIALIZED};
    std::unique_ptr<google_breakpad::ExceptionHandler> _handler;

#if defined(_WIN32)
    std::wstring _wabs_path;

#else
    std::string _abs_path;

#endif
};

} // namespace hj

#endif // CRASH_HPP