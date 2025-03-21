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

namespace libcpp
{

#if defined(_WIN32)
typedef bool(*dump_callback_t)(const wchar_t*, 
                               const wchar_t*, 
                               void*, 
                               EXCEPTION_POINTERS*, 
                               MDRawAssertionInfo*, 
                               bool);

static cbool default_dump_callback(const wchar_t* dump_dir,
                                   const wchar_t* minidump_id,
                                   void* context,
                                   EXCEPTION_POINTERS* exinfo,
                                   MDRawAssertionInfo* assertion,
                                   bool succeeded)
{
    return true;
}

#elif __APPLE__
typedef bool (*dump_callback_t)(const char *, const char *, void *, bool);

static bool default_dump_callback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
{
    return true;
}

#else
typedef bool (*dump_callback_t)(const google_breakpad::MinidumpDescriptor&, void*, bool );

static bool default_dump_callback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
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
    crash_handler(const char* abs_path)
        : _dump_cb{default_dump_callback}
    {
        set_local_path(abs_path);
    }
    crash_handler(const char* abs_path, const dump_callback_t cb)
        : _dump_cb{cb}
    {
        set_local_path(abs_path);
    }
    virtual ~crash_handler()
    {
    }

    inline void set_dump_callback(const dump_callback_t cb) { _dump_cb = cb; }

    inline void set_local_path(const char* abs_path) { set_local_path(std::string(abs_path)); }

    // NOTE: DON NOT SUPPORT RELATIVE PATH
    void set_local_path(const std::string& abs_path)
    {
        if (_handler != nullptr)
        {
            delete _handler;
        }

#if defined(_WIN32)
        std::wstring wabs_path(std::string(path).length(), L' ');
        std::copy(abs_path.begin(), abs_path.end(), wabs_path.begin());
        _handler = new google_breakpad::ExceptionHandler(abs_path, 
                                                         nullptr,                      // FilterCallback
                                                         _dump_cb, 
                                                         nullptr,                      // context
                                                         google_breakpad::ExceptionHandler::_handlerALL);
#elif __APPLE__
        _handler = new google_breakpad::ExceptionHandler(abs_path,
                                                         nullptr, // FilterCallback
                                                         _dump_cb, 
                                                         nullptr, // context
                                                         true, 
                                                         nullptr);
#else
        _handler = new google_breakpad::ExceptionHandler(google_breakpad::MinidumpDescriptor(abs_path),
                                                         NULL, // FilterCallback
                                                         _dump_cb,
                                                         NULL, // context
                                                         true,
                                                         -1);
#endif
    }

#if defined(_WIN32)
    // Reference to: https://www.cnblogs.com/cswuyg/p/3207576.html
    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI temp_set_unhandled_exception_filter( 
        LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
    {
        return NULL;
    }
#endif

    // Prevent exception handle by other application or crt.
    bool prevent_set_unhandled_exception_filter()
    {
#if defined(_WIN32)
        HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
        if (hKernel32 == NULL)
            return false;

        void *pOrgEntry = (void*)(::GetProcAddress(hKernel32, "SetUnhandledExceptionFilter"));
        if(pOrgEntry == NULL)
            return false;

        unsigned char newJump[5];
        DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
        dwOrgEntryAddr += 5; // jump instruction has 5 byte space.

        void *pNewFunc = (void*)(&temp_set_unhandled_exception_filter);
        DWORD dwNewEntryAddr = (DWORD)pNewFunc;
        DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

        newJump[0] = 0xE9;  // jump
        memcpy(&newJump[1], &dwRelativeAddr, sizeof(DWORD));
        SIZE_T bytesWritten;
        DWORD dwOldFlag, dwTempFlag;
        ::VirtualProtect(pOrgEntry, 5, PAGE_READWRITE, &dwOldFlag);
        BOOL bRet = ::WriteProcessMemory(::GetCurrentProcess(), pOrgEntry, newJump, 5, &bytesWritten);
        ::VirtualProtect(pOrgEntry, 5, dwOldFlag, &dwTempFlag);
        return bRet;
#else
        return true;
#endif
    }

public:
    static crash_handler* instance()
    {
        static crash_handler inst_;
        return &inst_;
    }

private:
    google_breakpad::ExceptionHandler* _handler = nullptr;
    dump_callback_t _dump_cb;
};

}

#endif