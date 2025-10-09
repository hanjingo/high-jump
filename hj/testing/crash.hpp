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
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#include <client/windows/handler/exception_handler.h>
#elif __APPLE__
#include <client/mac/handler/exception_handler.h>
#elif __linux__
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
    {
    }
    crash_handler(const char *abs_path)
        : _dump_cb{default_dump_callback}
    {
        set_local_path(abs_path);
    }
    crash_handler(const char *abs_path, const dump_callback_t cb)
        : _dump_cb{cb}
    {
        set_local_path(abs_path);
    }
    virtual ~crash_handler() {};

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
        std::wstring wabs_path(abs_path.begin(), abs_path.end());
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

  private:
    google_breakpad::ExceptionHandler *_handler = nullptr;
    dump_callback_t                    _dump_cb;
};

}

#endif