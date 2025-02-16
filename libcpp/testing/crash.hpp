#ifndef CRASH_HPP
#define CRASH_HPP

#include <set>
#include <string>
#include <iostream>
#include <functional>

#if defined(WIN32)
#include <windows.h>
#include <client/windows/handler/exception_handler.h>
#elif __APPLE__
#include <client/mac/handler/exception_handler.h>
#elif __linux__
#include <client/linux/handler/exception_handler.h>
#endif

namespace libcpp
{

#if defined(WIN32)
typedef bool(*dump_callback_t)(const wchar_t*, 
                               const wchar_t*, 
                               void*, 
                               EXCEPTION_POINTERS*, 
                               MDRawAssertionInfo*, 
                               bool);

bool default_dump_callback(const wchar_t* dump_dir,
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

bool default_dump_callback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
{
    return true;
}

#else
typedef bool (*dump_callback_t)(const google_breakpad::MinidumpDescriptor&, void*, bool );

bool default_dump_callback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
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
    crash_handler(const char* path)
        : _dump_cb{default_dump_callback}
    {
        set_local_path(path);
    }
    crash_handler(const char* path, const dump_callback_t cb)
        : _dump_cb{cb}
    {
        set_local_path(path);
    }
    virtual ~crash_handler()
    {
    }

    void set_dump_callback(const dump_callback_t cb)
    {
        _dump_cb = cb;
    }

    void set_local_path(const std::string& path)
    {
        set_local_path(path.c_str());
    }

    void set_local_path(const char* path)
    {
        if (_handler != nullptr)
        {
            delete _handler;
        }

#if defined(WIN32)
        std::wstring abs_path;
        _handler = new google_breakpad::ExceptionHandler(abs_path, 
                                                         nullptr,                      // FilterCallback
                                                         _dump_cb, 
                                                         nullptr,                      // context
                                                         google_breakpad::ExceptionHandler::_handlerALL);
#elif __APPLE__
        std::string abs_path;
        _handler = new google_breakpad::ExceptionHandler(abs_path,
                                                         nullptr, // FilterCallback
                                                         _dump_cb, 
                                                         nullptr, // context
                                                         true, 
                                                         nullptr);
#else
        std::string abs_path;
        _handler = new google_breakpad::ExceptionHandler(google_breakpad::MinidumpDescriptor(abs_path),
                                                         0, // FilterCallback
                                                         _dump_cb,
                                                         0, // context
                                                         true,
                                                         -1);
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

#if defined(WIN32)
// Reference to: https://www.cnblogs.com/cswuyg/p/3207576.html
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI temp_set_unhandled_exception_filter( 
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
    return NULL;
}

// Prevent exception handle by other application or crt.
bool prevent_set_unhandled_exception_filter()
{
    HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
    if (hKernel32 ==   NULL)
    {
        return false;
    }

    void *pOrgEntry = (void*)(::GetProcAddress(hKernel32, "SetUnhandledExceptionFilter"));
    if(pOrgEntry == NULL)
    {
        return false;
    }

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
}
#endif

}

#endif