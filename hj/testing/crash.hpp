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
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CRASH_HPP
#define CRASH_HPP

#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#include <client/windows/handler/exception_handler.h>
#elif __APPLE__
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <client/mac/handler/exception_handler.h>
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
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

class crash_handler
{
  public:
    crash_handler()
        : _dump_cb{default_dump_callback}
        , _handler{nullptr}
    {
    }
    crash_handler(const char *abs_path)
        : _dump_cb{default_dump_callback}
        , _handler{nullptr}
    {
        set_local_path(abs_path);
    }
    crash_handler(const char *abs_path, const dump_callback_t cb)
        : _dump_cb{cb}
        , _handler{nullptr}
    {
        set_local_path(abs_path);
    }
    virtual ~crash_handler()
    {
        if(_handler != nullptr)
        {
            delete _handler;
            _handler = nullptr;
        }
    };

    crash_handler(const crash_handler &)            = delete;
    crash_handler &operator=(const crash_handler &) = delete;
    crash_handler(crash_handler &&)                 = delete;
    crash_handler &operator=(crash_handler &&)      = delete;

    inline void set_dump_callback(const dump_callback_t cb) { _dump_cb = cb; }

    inline void set_local_path(const char *abs_path)
    {
        set_local_path(std::string(abs_path));
    }

    void set_local_path(const std::string &abs_path)
    {
        if(_handler != nullptr)
            delete _handler;

#if defined(_WIN32)
        int wlen =
            ::MultiByteToWideChar(CP_UTF8, 0, abs_path.c_str(), -1, NULL, 0);
        std::wstring wabs_path(wlen, 0);
        ::MultiByteToWideChar(CP_UTF8,
                              0,
                              abs_path.c_str(),
                              -1,
                              &wabs_path[0],
                              wlen);
        if(!wabs_path.empty() && wabs_path.back() == L'\0')
            wabs_path.pop_back();

        ::CreateDirectoryW(wabs_path.c_str(), NULL);
        _handler = new google_breakpad::ExceptionHandler(
            wabs_path,
            nullptr, // FilterCallback
            _dump_cb,
            nullptr, // context
            google_breakpad::ExceptionHandler::HANDLER_ALL);
#elif __APPLE__
        _handler =
            new google_breakpad::ExceptionHandler(abs_path,
                                                  nullptr, // FilterCallback
                                                  _dump_cb,
                                                  nullptr, // context
                                                  true,
                                                  nullptr);
#else
        _handler = new google_breakpad::ExceptionHandler(
            google_breakpad::MinidumpDescriptor(abs_path),
            nullptr, // FilterCallback
            _dump_cb,
            nullptr, // context
            true,
            -1);
#endif
    }

#if defined(_WIN32)
    // Reference to: https://www.cnblogs.com/cswuyg/p/3207576.html
    static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
    temp_set_unhandled_exception_filter(
        LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
    {
        (void) lpTopLevelExceptionFilter;
        return NULL;
    }
#endif

    bool prevent_set_unhandled_exception_filter()
    {
#if defined(_WIN32) && !defined(_WIN64) // Only for 32-bit Windows
        HMODULE hKernel32 = LoadLibraryW(
            L"kernel32.dll"); // Use LoadLibraryW for wide-character strings
        if(hKernel32 == NULL)
            return false;

        void *pOrgEntry =
            (void *) (::GetProcAddress(hKernel32,
                                       "SetUnhandledExceptionFilter"));
        if(pOrgEntry == NULL)
            return false;

        unsigned char newJump[5];
        ULONG_PTR     dwOrgEntryAddr = reinterpret_cast<ULONG_PTR>(pOrgEntry);
        dwOrgEntryAddr += 5; // jump instruction has 5 byte space.

        void *pNewFunc =
            (void *) (&crash_handler::temp_set_unhandled_exception_filter);
        ULONG_PTR dwNewEntryAddr = reinterpret_cast<ULONG_PTR>(pNewFunc);
        LONG      dwRelativeAddr =
            static_cast<LONG>(dwNewEntryAddr - dwOrgEntryAddr);

        newJump[0] = 0xE9; // jump
        memcpy(&newJump[1], &dwRelativeAddr, sizeof(LONG));
        SIZE_T bytesWritten;
        DWORD  dwOldFlag, dwTempFlag;
        ::VirtualProtect(pOrgEntry, 5, PAGE_READWRITE, &dwOldFlag);
        BOOL bRet = ::WriteProcessMemory(::GetCurrentProcess(),
                                         pOrgEntry,
                                         newJump,
                                         5,
                                         &bytesWritten);
        ::VirtualProtect(pOrgEntry, 5, dwOldFlag, &dwTempFlag);
        return bRet;
#else
        return true;
#endif
    }

  public:
    static crash_handler *instance()
    {
        static crash_handler inst;
        return &inst;
    }

    static void print(const char *content, const char *path = "crash.log")
    {
        if(!content || !path)
            return;

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

        DWORD bytesWritten;
        if(timeLen > 0)
            ::WriteFile(hFile, timeBuf, timeLen, &bytesWritten, NULL);

        int contentLen = 0;
        while(content[contentLen])
            contentLen++;

        ::WriteFile(hFile, content, contentLen, &bytesWritten, NULL);
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
                auto ret = ::write(fd, timeBuf, timeLen);
                (void) ret;
            }
        }

        size_t contentLen = 0;
        while(content[contentLen])
            contentLen++;

        auto ret1 = ::write(fd, content, contentLen);
        (void) ret1;

        auto ret2 = ::write(fd, "\n", 1);
        (void) ret2;

        ::close(fd);
#endif
    }

  private:
    google_breakpad::ExceptionHandler *_handler = nullptr;
    dump_callback_t                    _dump_cb;
};

}

#endif