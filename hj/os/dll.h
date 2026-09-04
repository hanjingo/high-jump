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
#ifndef DLL_H
#define DLL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wchar.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#define DLL_EXT ".dll"
#define DLL_PREFIX ""
#elif defined(__APPLE__)
#define DLL_EXT ".dylib"
#define DLL_PREFIX "lib"
#elif defined(__linux__) || defined(__ANDROID__)
#define DLL_EXT ".so"
#define DLL_PREFIX "lib"
#else
#define DLL_EXT ""
#define DLL_PREFIX ""
#pragma message("WARNING: UNKNOWN DYNAMIC LINK LIBRARY FILE EXTENSION.")
#endif

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif

#ifdef __cplusplus
#define C_STYLE_EXPORT extern "C" DLL_EXPORT
#define C_STYLE_IMPORT extern "C" DLL_IMPORT
#else
#define C_STYLE_EXPORT DLL_EXPORT
#define C_STYLE_IMPORT DLL_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int dll_mode_t;

#define DLL_MODE_DEFAULT 0
#define DLL_MODE_RTLD_NOW (1 << 0)
#define DLL_MODE_RTLD_GLOBAL (1 << 1)
#define DLL_MODE_RTLD_LOCAL (1 << 2)
#define DLL_MODE_RTLD_NOLOAD (1 << 3)
#define DLL_MODE_RTLD_DEEPBIND (1 << 4)

#define DLL_MODE_SEARCH_ALTERED_PATH                                           \
    (1 << 8) // Windows Only: LOAD_WITH_ALTERED_SEARCH_PATH
#define DLL_MODE_SEARCH_SYSTEM32                                               \
    (1 << 9) // Windows Only: LOAD_LIBRARY_SEARCH_SYSTEM32

// define sdk common api
typedef struct sdk_context
{
    uint64_t sz;
    void    *user_data;
    void (*cb)(void *);
} sdk_context;

typedef void (*sdk_callback)(void *);
typedef void (*sdk_api)(sdk_context *);

// thread safe error buffer for dll operations
static inline char *dll_get_last_err(void)
{
#if defined(_MSC_VER)
    static __declspec(thread) char err_buf[512];
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    static _Thread_local char err_buf[512];
#elif defined(__GNUC__) || defined(__clang__)
    static __thread char err_buf[512];
#else
    static char err_buf[512]; // Fallback
#endif
    return err_buf;
}

static inline void dll_set_last_err(const char *msg)
{
    char *buf = dll_get_last_err();
    if(msg)
    {
#if defined(_MSC_VER)
        strncpy_s(buf, 512, msg, _TRUNCATE);
#else
        strncpy(buf, msg, 511);
        buf[511] = '\0';
#endif
    } else
    {
        buf[0] = '\0';
    }
}

// ---------------------------------------------------------------------------
// Common API for Dynamic Link Library (DLL) Operations
// ---------------------------------------------------------------------------
static inline void dll_clear_error(void)
{
    char *buf = dll_get_last_err();
    buf[0]    = '\0';

#if defined(_WIN32)
    SetLastError(0);
#else
    dlerror();
#endif
}

static inline const char *dll_error(void)
{
    char *buf = dll_get_last_err();
    if(buf[0] != '\0')
        return buf;

#if defined(_WIN32)
    DWORD err_code = GetLastError();
    if(err_code == 0)
        return "";

    DWORD len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
                                   | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               err_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               buf,
                               512,
                               NULL);

    if(len == 0)
        return "Unknown system error";

    while(len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
        buf[--len] = '\0';

    return buf;
#else
    const char *err = dlerror();
    return err ? err : "";
#endif
}

static inline void *dll_open(const char *filename, dll_mode_t mode)
{
    dll_clear_error();

    if(!filename)
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        dll_set_last_err("Invalid argument: filename is null");
        return NULL;
    }

#if defined(_WIN32)
    int wlen = MultiByteToWideChar(CP_UTF8,
                                   MB_ERR_INVALID_CHARS,
                                   filename,
                                   -1,
                                   NULL,
                                   0);
    if(wlen <= 0)
    {
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
        dll_set_last_err("Invalid UTF-8 sequence in filename");
        return NULL;
    }

    wchar_t  stack_buf[256];
    wchar_t *wpath = stack_buf;
    if((size_t) wlen > sizeof(stack_buf) / sizeof(wchar_t))
    {
        wpath = (wchar_t *) malloc((size_t) wlen * sizeof(wchar_t));
        if(!wpath)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            dll_set_last_err("Out of memory during path conversion");
            return NULL;
        }
    }

    if(MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, wlen) == 0)
    {
        if(wpath != stack_buf)
            free(wpath);

        dll_set_last_err("Failed to convert filename to UTF-16");
        return NULL;
    }

    wchar_t        full_path[MAX_PATH];
    DWORD          ret = GetFullPathNameW(wpath, MAX_PATH, full_path, NULL);
    const wchar_t *target_path =
        (ret > 0 && ret < MAX_PATH) ? full_path : wpath;

    if(mode & DLL_MODE_RTLD_NOLOAD)
    {
        HMODULE hmod = NULL;
        BOOL    ok   = GetModuleHandleExW(0, target_path, &hmod);
        if(!ok)
        {
            const wchar_t *file_name = wcsrchr(wpath, L'/');
            const wchar_t *bslash    = wcsrchr(wpath, L'\\');
            if(bslash > file_name)
                file_name = bslash;
            file_name = file_name ? (file_name + 1) : wpath;

            ok = GetModuleHandleExW(0, file_name, &hmod);
        }

        if(wpath != stack_buf)
            free(wpath);

        if(!ok)
        {
            SetLastError(ERROR_MOD_NOT_FOUND);
            dll_set_last_err("Module not currently loaded (RTLD_NOLOAD)");
            return NULL;
        }

        return (void *) hmod;
    }

    DWORD dwFlags = 0;
    if(mode & DLL_MODE_SEARCH_ALTERED_PATH)
        dwFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
    if(mode & DLL_MODE_SEARCH_SYSTEM32)
        dwFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;

    HMODULE hmod = LoadLibraryExW(target_path, NULL, dwFlags);
    if(wpath != stack_buf)
        free(wpath);

    return (void *) hmod;

#else
    int posix_flags = 0;
    if(mode & DLL_MODE_RTLD_NOW)
        posix_flags |= RTLD_NOW;
    else
        posix_flags |= RTLD_LAZY;

    if(mode & DLL_MODE_RTLD_GLOBAL)
        posix_flags |= RTLD_GLOBAL;
    if(mode & DLL_MODE_RTLD_LOCAL)
        posix_flags |= RTLD_LOCAL;
    if(mode & DLL_MODE_RTLD_NOLOAD)
        posix_flags |= RTLD_NOLOAD;
#if defined(RTLD_DEEPBIND)
    if(mode & DLL_MODE_RTLD_DEEPBIND)
        posix_flags |= RTLD_DEEPBIND;
#endif

    void *handle = dlopen(filename, posix_flags);
    if(!handle && (mode & DLL_MODE_RTLD_NOLOAD))
    {
        const char *err = dlerror();
        if(!err || err[0] == '\0')
            dll_set_last_err("Module not currently loaded (RTLD_NOLOAD)");
    }
    return handle;
#endif
}

static inline void *dll_get(void *handler, const char *symbol)
{
    dll_clear_error();

    if(!handler || !symbol)
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        dll_set_last_err("Invalid argument: handle or symbol is null");
        return NULL;
    }

#if defined(_WIN32)
    void *sym = (void *) GetProcAddress((HMODULE) handler, symbol);
    return sym;
#else
    void       *sym = dlsym(handler, symbol);
    const char *err = dlerror();
    if(err != NULL)
        return NULL;

    return sym;
#endif
}

static inline int dll_close(void *handle)
{
    dll_clear_error();
    if(!handle)
    {
#if defined(_WIN32)
        SetLastError(ERROR_INVALID_PARAMETER);
#endif
        dll_set_last_err("Invalid argument: handle is null");
        return -1;
    }

#if defined(_WIN32)
    BOOL ret = FreeLibrary((HMODULE) handle);
    return ret ? 0 : -1;
#else
    int ret = dlclose(handle);
    return ret;
#endif
}

#ifdef __cplusplus
}
#endif

#endif // DLL_H