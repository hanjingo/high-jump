/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have registered a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CRASH_HPP
#define CRASH_HPP

#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>

#if defined(_WIN32)
#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <BaseTsd.h>
#include <signal.h>
#include <intrin.h>
typedef SSIZE_T ssize_t;
#include <client/windows/handler/exception_handler.h>
#elif __APPLE__
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <client/mac/handler/exception_handler.h>
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <client/linux/handler/exception_handler.h>
#endif

namespace hj
{

#if defined(_WIN32)
typedef bool (*dump_callback_t)(const wchar_t *,
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
    (void) succeeded;
    return true;
}

#elif __APPLE__
typedef bool (*dump_callback_t)(const char *, const char *, void *, bool);

static inline void crash_sigabrt_handler(int sig)
{
    (void) sig;
    std::abort();
}

static bool default_dump_callback(const char *dump_dir,
                                  const char *minidump_id,
                                  void       *context,
                                  bool        succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    (void) succeeded;
    return true;
}

#else
typedef bool (*dump_callback_t)(const google_breakpad::MinidumpDescriptor &,
                                void *,
                                bool);

static inline void crash_sigabrt_handler(int sig)
{
    (void) sig;
    std::abort();
}

static bool
default_dump_callback(const google_breakpad::MinidumpDescriptor &descriptor,
                      void                                      *context,
                      bool                                       succeeded)
{
    (void) descriptor;
    (void) context;
    (void) succeeded;
    return true;
}
#endif

static inline bool crash_safe_write_all(int fd, const char *buf, size_t count)
{
#if defined(_WIN32)
    (void) fd;
    (void) buf;
    (void) count;
    return false;
#else
    size_t total_written = 0;
    while(total_written < count)
    {
        ssize_t written =
            ::write(fd, buf + total_written, count - total_written);
        if(written < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            return false;
        }
        if(written == 0)
        {
            break;
        }
        total_written += static_cast<size_t>(written);
    }
    return total_written == count;
#endif
}

static inline void crash_print(const char *content,
                               const char *path = "crash.log")
{
    if(!content || !path)
        return;

    size_t content_len = 0;
    while(content[content_len])
    {
        content_len++;
    }

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileA(path,
                                 FILE_APPEND_DATA,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

    if(hFile == INVALID_HANDLE_VALUE)
        return;

    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char timeBuf[64];
    int  timeLen = ::wsprintfA(timeBuf,
                               "%04d-%02d-%02d %02d:%02d:%02d : ",
                               st.wYear,
                               st.wMonth,
                               st.wDay,
                               st.wHour,
                               st.wMinute,
                               st.wSecond);

    DWORD bytesWritten = 0;
    if(timeLen > 0)
    {
        ::WriteFile(hFile,
                    timeBuf,
                    static_cast<DWORD>(timeLen),
                    &bytesWritten,
                    NULL);
    }

    if(content_len > 0)
    {
        ::WriteFile(hFile,
                    content,
                    static_cast<DWORD>(content_len),
                    &bytesWritten,
                    NULL);
    }
    ::WriteFile(hFile, "\r\n", 2, &bytesWritten, NULL);
    ::CloseHandle(hFile);

#else // Linux / macOS (POSIX)
    int fd = ::open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd < 0)
        return;

    struct timespec ts;
    struct tm       tm_info;
    if(::clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        ::localtime_r(&ts.tv_sec, &tm_info);
        char   timeBuf[64];
        size_t timeLen = ::strftime(timeBuf,
                                    sizeof(timeBuf),
                                    "%Y-%m-%d %H:%M:%S : ",
                                    &tm_info);
        if(timeLen > 0)
        {
            crash_safe_write_all(fd, timeBuf, timeLen);
        }
    }

    if(content_len > 0)
    {
        crash_safe_write_all(fd, content, content_len);
    }
    crash_safe_write_all(fd, "\n", 1);

    ::close(fd);
#endif
}

#if defined(_WIN32)
static inline bool crash_ensure_directories_exist(const std::wstring &wpath)
{
    if(wpath.empty())
        return false;

    std::wstring temp = wpath;
    for(size_t i = 0; i < temp.size(); ++i)
    {
        if(temp[i] == L'\\' || temp[i] == L'/')
        {
            wchar_t prev = temp[i + 1];
            temp[i + 1]  = L'\0';
            if(!::CreateDirectoryW(temp.c_str(), NULL))
            {
                DWORD err = ::GetLastError();
                if(err != ERROR_ALREADY_EXISTS)
                {
                    // ignore error, continue to next
                }
            }
            temp[i + 1] = prev;
        }
    }

    if(!::CreateDirectoryW(temp.c_str(), NULL))
    {
        DWORD err = ::GetLastError();
        if(err != ERROR_ALREADY_EXISTS)
        {
            return false;
        }
    }

    DWORD attr = ::GetFileAttributesW(temp.c_str());
    if(attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return false;
    }
    return true;
}
#else
static inline bool crash_ensure_directories_exist(const std::string &path_str)
{
    if(path_str.empty())
        return false;

    std::string temp = path_str;
    for(size_t i = 1; i < temp.size(); ++i)
    {
        if(temp[i] == '/')
        {
            temp[i] = '\0';
            if(::mkdir(temp.c_str(), 0755) != 0)
            {
                if(errno != EEXIST)
                {
                    // ignore error, continue to next
                }
            }
            temp[i] = '/';
        }
    }

    if(::mkdir(temp.c_str(), 0755) != 0)
    {
        if(errno != EEXIST)
            return false;
    }

    struct stat st;
    if(::stat(temp.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
        return false;

    return true;
}
#endif

class crash_handler
{
  public:
    crash_handler()
        : _handler{nullptr}
#if defined(_WIN32)
        , _pOrgEntry{nullptr}
        , _patchSize{0}
        , _is_patched{false}
#endif
    {
    }

    explicit crash_handler(const char           *abs_path,
                           const dump_callback_t cb = nullptr)
        : _handler{nullptr}
#if defined(_WIN32)
        , _pOrgEntry{nullptr}
        , _patchSize{0}
        , _is_patched{false}
#endif
    {
        if(abs_path)
        {
            init(std::string(abs_path), cb);
        }
    }

    ~crash_handler()
    {
#if defined(_WIN32)
        if(_is_patched && _pOrgEntry != nullptr && _patchSize > 0)
        {
            DWORD dwOldFlag = 0, dwTempFlag = 0;
            if(::VirtualProtect(_pOrgEntry,
                                _patchSize,
                                PAGE_EXECUTE_READWRITE,
                                &dwOldFlag))
            {
                std::memcpy(_pOrgEntry, _origBytes, _patchSize);
                ::FlushInstructionCache(GetCurrentProcess(),
                                        _pOrgEntry,
                                        _patchSize);
                ::VirtualProtect(_pOrgEntry,
                                 _patchSize,
                                 dwOldFlag,
                                 &dwTempFlag);
            }
        }
#endif
    }

    crash_handler(const crash_handler &)            = delete;
    crash_handler &operator=(const crash_handler &) = delete;
    crash_handler(crash_handler &&)                 = delete;
    crash_handler &operator=(crash_handler &&)      = delete;

    inline void init(const char *abs_path, const dump_callback_t cb = nullptr)
    {
        if(abs_path)
        {
            init(std::string(abs_path), cb);
        }
    }

    void init(const std::string &abs_path, dump_callback_t cb = nullptr)
    {
        if(abs_path.empty())
        {
            return;
        }

        std::call_once(_init_flag, [this, &abs_path, cb]() {
            dump_callback_t final_cb = cb ? cb : default_dump_callback;

#if defined(_WIN32)
            int wlen   = ::MultiByteToWideChar(CP_UTF8,
                                               0,
                                               abs_path.c_str(),
                                               -1,
                                               NULL,
                                               0);
            _wabs_path = std::wstring(wlen, 0);
            ::MultiByteToWideChar(CP_UTF8,
                                  0,
                                  abs_path.c_str(),
                                  -1,
                                  &_wabs_path[0],
                                  wlen);
            if(!_wabs_path.empty() && _wabs_path.back() == L'\0')
                _wabs_path.pop_back();

            if(!crash_ensure_directories_exist(_wabs_path))
            {
                return;
            }

            _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                _wabs_path,
                nullptr, // FilterCallback
                final_cb,
                nullptr, // context
                google_breakpad::ExceptionHandler::HANDLER_ALL);

#else
            if(!crash_ensure_directories_exist(abs_path))
            {
                return;
            }

            _abs_path = abs_path;
#if __APPLE__
            _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                _abs_path,
                nullptr, // FilterCallback
                final_cb,
                nullptr, // context
                true,
                nullptr);
#else
            _handler = std::make_unique<google_breakpad::ExceptionHandler>(
                google_breakpad::MinidumpDescriptor(_abs_path),
                nullptr, // FilterCallback
                final_cb,
                nullptr, // context
                true,
                -1);
#endif
#endif
        });
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

    bool write_minidump()
    {
        if(_handler)
            return _handler->WriteMinidump();

        return false;
    }

  public:
    static crash_handler *instance()
    {
        static crash_handler inst;
        return &inst;
    }

  private:
    std::once_flag                                     _init_flag;
    std::unique_ptr<google_breakpad::ExceptionHandler> _handler;

#if defined(_WIN32)
    void         *_pOrgEntry;
    SIZE_T        _patchSize;
    unsigned char _origBytes[16];
    bool          _is_patched;

    std::wstring _wabs_path;
#else
    std::string _abs_path;
#endif
};

} // namespace hj

#endif