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
#ifndef MAINBOARD_H
#define MAINBOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_MAINBOARD_API
#if defined(HJ_MAINBOARD_STATIC)
#define HJ_MAINBOARD_API static inline
#else
#define HJ_MAINBOARD_API extern
#endif
#endif

typedef enum
{
    HJ_MAINBOARD_OK                           = 0,
    HJ_MAINBOARD_ERR_UNKNOWN                  = -1,
    HJ_MAINBOARD_ERR_NOT_SUPPORTED            = -2,
    HJ_MAINBOARD_ERR_INVALID_ARG              = -3,
    HJ_MAINBOARD_ERR_INTERNAL                 = -4,
    HJ_MAINBOARD_ERR_ALLOCATION_MEMORY_FAILED = -5,
    HJ_MAINBOARD_ERR_OPEN_FILE_FAILED         = -6,
    HJ_MAINBOARD_ERR_READ_INFO_FAILED         = -7,
    HJ_MAINBOARD_ERR_SYSCTL_FAILED            = -8,
    HJ_MAINBOARD_ERR_SYSCONF_FAILED           = -9
} hj_mainboard_err_t;

// ------------------------ Mainboard API Declarations ------------------------

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_model(char *buf, size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_vendor(char *buf, size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_serial_num(char  *buf,
                                                            size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_bios_version(char  *buf,
                                                              size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_chipset(char  *buf,
                                                         size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t
hj_mainboard_memory_slots(unsigned int *slots);

HJ_MAINBOARD_API hj_mainboard_err_t
hj_mainboard_expansion_slots(unsigned int *slots);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_manufacturer_name(char  *buf,
                                                                   size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_product_name(char  *buf,
                                                              size_t size);

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_version(uint8_t *major,
                                                         uint8_t *minor,
                                                         uint8_t *patch);

#ifdef __cplusplus
}
#endif

#endif // MAINBOARD_H

// --------------------- Implementation -------------------------
#if (defined(HJ_MAINBOARD_IMPL) || defined(HJ_MAINBOARD_STATIC))               \
    && !defined(HJ_MAINBOARD_IMPL_DONE)
#define HJ_MAINBOARD_IMPL_DONE

#ifdef _WIN32
#define COBJMACROS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#elif defined(__linux__)
#include <unistd.h>
#include <ctype.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------ Helper Utilities ------------------------

HJ_MAINBOARD_API int hj_parse_version_string(const char *str,
                                             uint8_t    *major,
                                             uint8_t    *minor,
                                             uint8_t    *patch)
{
    if(!str || !major || !minor || !patch)
        return -1;
    *major = *minor = *patch = 0;

    const char *p = str;
    while(*p && !(*p >= '0' && *p <= '9'))
        p++;
    if(!*p)
        return -1;

    unsigned int ma = 0, mi = 0, pa = 0;
    int          count = sscanf(p, "%u.%u.%u", &ma, &mi, &pa);
    if(count >= 1)
    {
        if(ma > 255 || mi > 255 || pa > 255)
            return -1;

        *major = (uint8_t) ma;
        if(count >= 2)
            *minor = (uint8_t) mi;
        if(count >= 3)
            *patch = (uint8_t) pa;
        return 0;
    }
    return -1;
}

#ifdef _WIN32
#if defined(_MSC_VER)
#define HJ_THREAD_LOCAL __declspec(thread)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define HJ_THREAD_LOCAL _Thread_local
#else
#define HJ_THREAD_LOCAL __thread
#endif

static HJ_THREAD_LOCAL BOOL g_com_initialized = FALSE;
static LONG                 g_sec_initialized = 0;

HJ_MAINBOARD_API int hj_ensure_com_initialized(void)
{
    if(!g_com_initialized)
    {
        HRESULT hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if(FAILED(hres) && hres != RPC_E_CHANGED_MODE)
            return -1;
        g_com_initialized = TRUE;
    }

    if(InterlockedCompareExchange(&g_sec_initialized, 1, 0) == 0)
    {
        CoInitializeSecurity(NULL,
                             -1,
                             NULL,
                             NULL,
                             RPC_C_AUTHN_LEVEL_DEFAULT,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE,
                             NULL);
    }
    return 0;
}

HJ_MAINBOARD_API int hj_get_wmi_property(const char *wmi_class,
                                         const char *prop_name,
                                         char       *buf,
                                         size_t      size)
{
    HRESULT               hres        = S_OK;
    IWbemLocator         *pLoc        = NULL;
    IWbemServices        *pSvc        = NULL;
    IEnumWbemClassObject *pEnumerator = NULL;
    IWbemClassObject     *pclsObj     = NULL;
    int                   status      = -1;
    ULONG                 uReturn     = 0;
    VARIANT               vtProp;
    wchar_t               query[256];
    wchar_t               wprop_name[128];

    if(!wmi_class || !prop_name || !buf || size == 0)
        return -1;
    buf[0] = '\0';

    if(hj_ensure_com_initialized() != 0)
        return -1;

    VariantInit(&vtProp);

#ifdef __cplusplus
    hres = CoCreateInstance(CLSID_WbemLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (LPVOID *) &pLoc);
#else
    hres = CoCreateInstance(&CLSID_WbemLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            &IID_IWbemLocator,
                            (LPVOID *) &pLoc);
#endif

    if(FAILED(hres) || !pLoc)
        goto cleanup;

#ifdef __cplusplus
    hres = pLoc->ConnectServer((BSTR) L"ROOT\\CIMV2",
                               NULL,
                               NULL,
                               0,
                               0,
                               0,
                               NULL,
                               &pSvc);
#else
    hres = pLoc->lpVtbl->ConnectServer(pLoc,
                                       (BSTR) L"ROOT\\CIMV2",
                                       NULL,
                                       NULL,
                                       0,
                                       0,
                                       0,
                                       NULL,
                                       &pSvc);
#endif
    if(FAILED(hres) || !pSvc)
        goto cleanup;

#ifdef __cplusplus
    hres = CoSetProxyBlanket((IUnknown *) pSvc,
                             RPC_C_AUTHN_WINNT,
                             RPC_C_AUTHZ_NONE,
                             NULL,
                             RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE);
#else
    hres = CoSetProxyBlanket((IUnknown *) pSvc,
                             RPC_C_AUTHN_WINNT,
                             RPC_C_AUTHZ_NONE,
                             NULL,
                             RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE);
#endif
    if(FAILED(hres))
        goto cleanup;

    swprintf_s(query, 256, L"SELECT %S FROM %S", prop_name, wmi_class);

#ifdef __cplusplus
    hres =
        pSvc->ExecQuery((BSTR) L"WQL",
                        (BSTR) query,
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator);
#else
    hres = pSvc->lpVtbl->ExecQuery(pSvc,
                                   (BSTR) L"WQL",
                                   (BSTR) query,
                                   WBEM_FLAG_FORWARD_ONLY
                                       | WBEM_FLAG_RETURN_IMMEDIATELY,
                                   NULL,
                                   &pEnumerator);
#endif
    if(FAILED(hres) || !pEnumerator)
        goto cleanup;

#ifdef __cplusplus
    hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
#else
    hres = pEnumerator->lpVtbl->Next(pEnumerator,
                                     WBEM_INFINITE,
                                     1,
                                     &pclsObj,
                                     &uReturn);
#endif
    if(0 == uReturn || !pclsObj)
        goto cleanup;

    size_t converted = mbstowcs(wprop_name,
                                prop_name,
                                sizeof(wprop_name) / sizeof(wchar_t) - 1);
    if(converted == (size_t) -1)
        goto cleanup;
    wprop_name[converted] = L'\0';

#ifdef __cplusplus
    hres = pclsObj->Get(wprop_name, 0, &vtProp, 0, 0);
#else
    hres = pclsObj->lpVtbl->Get(pclsObj, wprop_name, 0, &vtProp, 0, 0);
#endif

    if(SUCCEEDED(hres))
    {
        if(vtProp.vt == VT_BSTR && vtProp.bstrVal)
        {
            wcstombs(buf, vtProp.bstrVal, size - 1);
            buf[size - 1] = '\0';
            status        = 0;
        } else if(vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)
        {
            snprintf(buf, size, "%ld", (long) vtProp.lVal);
            status = 0;
        }
    }
    VariantClear(&vtProp);

cleanup:
    if(pclsObj)
    {
#ifdef __cplusplus
        pclsObj->Release();
#else
        pclsObj->lpVtbl->Release(pclsObj);
#endif
    }
    if(pEnumerator)
    {
#ifdef __cplusplus
        pEnumerator->Release();
#else
        pEnumerator->lpVtbl->Release(pEnumerator);
#endif
    }
    if(pSvc)
    {
#ifdef __cplusplus
        pSvc->Release();
#else
        pSvc->lpVtbl->Release(pSvc);
#endif
    }
    if(pLoc)
    {
#ifdef __cplusplus
        pLoc->Release();
#else
        pLoc->lpVtbl->Release(pLoc);
#endif
    }

    return status;
}
#endif // _WIN32

#ifdef __linux__
HJ_MAINBOARD_API int hj_read_sysfs(const char *path, char *buf, size_t size)
{
    if(!path || !buf || size == 0)
        return -1;
    buf[0] = '\0';

    FILE *f = fopen(path, "r");
    if(!f)
        return -1;

    if(!fgets(buf, (int) size, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

HJ_MAINBOARD_API int hj_get_linux_chipset(char *buf, size_t size)
{
    char vendor_str[32] = {0};
    char device_str[32] = {0};

    if(hj_read_sysfs("/sys/bus/pci/devices/0000:00:00.0/vendor",
                     vendor_str,
                     sizeof(vendor_str))
           == 0
       && hj_read_sysfs("/sys/bus/pci/devices/0000:00:00.0/device",
                        device_str,
                        sizeof(device_str))
              == 0)
    {
        unsigned long v_id = strtoul(vendor_str, NULL, 0);
        unsigned long d_id = strtoul(device_str, NULL, 0);

        const char *v_name = "PCI";
        if(v_id == 0x8086)
            v_name = "Intel";
        else if(v_id == 0x1022)
            v_name = "AMD";
        else if(v_id == 0x10de)
            v_name = "NVIDIA";

        snprintf(buf,
                 size,
                 "%s Host Bridge [%04lx:%04lx]",
                 v_name,
                 v_id & 0xffff,
                 d_id & 0xffff);
        return 0;
    }
    return -1;
}
#endif

#ifdef __APPLE__
HJ_MAINBOARD_API int hj_sysctl_string(const char *name, char *buf, size_t size)
{
    if(!name || !buf || size == 0)
        return -1;
    buf[0] = '\0';

    size_t len = size - 1;
    if(sysctlbyname(name, buf, &len, NULL, 0) != 0)
        return -1;

    buf[len] = '\0';
    return 0;
}
#endif

// ------------------------ API Implementations ------------------------

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_model(char *buf, size_t size)
{
    if(!buf || size == 0)
        return HJ_MAINBOARD_ERR_INVALID_ARG;

#ifdef _WIN32
    return hj_get_wmi_property("Win32_BaseBoard", "Product", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__linux__)
    return hj_read_sysfs("/sys/class/dmi/id/board_name", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__APPLE__)
    return hj_sysctl_string("hw.model", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_SYSCTL_FAILED;
#else
    buf[0] = '\0';
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_vendor(char *buf, size_t size)
{
    if(!buf || size == 0)
        return HJ_MAINBOARD_ERR_INVALID_ARG;

#ifdef _WIN32
    return hj_get_wmi_property("Win32_BaseBoard", "Manufacturer", buf, size)
                   == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__linux__)
    return hj_read_sysfs("/sys/class/dmi/id/board_vendor", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__APPLE__)
    if(size < sizeof("Apple Inc."))
        return HJ_MAINBOARD_ERR_INVALID_ARG;
    snprintf(buf, size, "Apple Inc.");
    return HJ_MAINBOARD_OK;
#else
    buf[0] = '\0';
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_serial_num(char  *buf,
                                                            size_t size)
{
    if(!buf || size == 0)
        return HJ_MAINBOARD_ERR_INVALID_ARG;
    buf[0] = '\0';

#ifdef _WIN32
    return hj_get_wmi_property("Win32_BaseBoard", "SerialNumber", buf, size)
                   == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__linux__)
    return hj_read_sysfs("/sys/class/dmi/id/board_serial", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__APPLE__)
    io_service_t platformExpert = IOServiceGetMatchingService(
        kIOMainPortDefault,
        IOServiceMatching("IOPlatformExpertDevice"));
    if(!platformExpert)
        return HJ_MAINBOARD_ERR_READ_INFO_FAILED;

    CFTypeRef serialAsCFString =
        IORegistryEntryCreateCFProperty(platformExpert,
                                        CFSTR(kIOPlatformSerialNumberKey),
                                        kCFAllocatorDefault,
                                        0);
    IOObjectRelease(platformExpert);

    if(!serialAsCFString)
        return HJ_MAINBOARD_ERR_READ_INFO_FAILED;

    Boolean res = CFStringGetCString((CFStringRef) serialAsCFString,
                                     buf,
                                     size,
                                     kCFStringEncodingUTF8);
    CFRelease(serialAsCFString);
    return res ? HJ_MAINBOARD_OK : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#else
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_bios_version(char  *buf,
                                                              size_t size)
{
    if(!buf || size == 0)
        return HJ_MAINBOARD_ERR_INVALID_ARG;
    buf[0] = '\0';

#ifdef _WIN32
    return hj_get_wmi_property("Win32_BIOS", "SMBIOSBIOSVersion", buf, size)
                   == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__linux__)
    return hj_read_sysfs("/sys/class/dmi/id/bios_version", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__APPLE__)
    io_service_t platformExpert = IOServiceGetMatchingService(
        kIOMainPortDefault,
        IOServiceMatching("IOPlatformExpertDevice"));
    if(platformExpert)
    {
        CFDataRef romRef =
            (CFDataRef) IORegistryEntryCreateCFProperty(platformExpert,
                                                        CFSTR("rom-revision"),
                                                        kCFAllocatorDefault,
                                                        0);
        IOObjectRelease(platformExpert);

        if(romRef)
        {
            const char *bytes = (const char *) CFDataGetBytePtr(romRef);
            CFIndex     len   = CFDataGetLength(romRef);
            if(bytes && len > 0)
            {
                size_t copy_len =
                    (size_t) len < (size - 1) ? (size_t) len : (size - 1);
                memcpy(buf, bytes, copy_len);
                buf[copy_len] = '\0';
                CFRelease(romRef);
                return HJ_MAINBOARD_OK;
            }
            CFRelease(romRef);
        }
    }
    return hj_sysctl_string("kern.version", buf, size) == 0
               ? HJ_MAINBOARD_OK
               : HJ_MAINBOARD_ERR_SYSCTL_FAILED;
#else
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_chipset(char *buf, size_t size)
{
    if(!buf || size == 0)
        return HJ_MAINBOARD_ERR_INVALID_ARG;
    buf[0] = '\0';

    int ret = -1;
#ifdef _WIN32
    ret = hj_get_wmi_property("Win32_Chipset", "Name", buf, size);
#elif defined(__linux__)
    ret = hj_get_linux_chipset(buf, size);
#elif defined(__APPLE__)
    snprintf(buf, size, "Apple SoC/Chipset");
    ret = 0;
#endif

    if(ret != 0 || buf[0] == '\0')
    {
        strncpy(buf, "Unknown", size - 1);
        buf[size - 1] = '\0';
        return HJ_MAINBOARD_ERR_READ_INFO_FAILED;
    }
    return HJ_MAINBOARD_OK;
}

HJ_MAINBOARD_API hj_mainboard_err_t
hj_mainboard_memory_slots(unsigned int *slots)
{
    if(!slots)
        return HJ_MAINBOARD_ERR_INVALID_ARG;

#ifdef _WIN32
    char buf[128];
    if(hj_get_wmi_property("Win32_PhysicalMemoryArray",
                           "MemoryDevices",
                           buf,
                           sizeof(buf))
       == 0)
    {
        *slots = (unsigned int) atoi(buf);
        return HJ_MAINBOARD_OK;
    }
    return HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#elif defined(__linux__)
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#elif defined(__APPLE__)
    int    val = 0;
    size_t len = sizeof(val);
    if(sysctlbyname("hw.memslots", &val, &len, NULL, 0) == 0)
    {
        *slots = (unsigned int) (val < 0 ? 0 : val);
        return HJ_MAINBOARD_OK;
    }
    return HJ_MAINBOARD_ERR_SYSCTL_FAILED;
#else
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t
hj_mainboard_expansion_slots(unsigned int *slots)
{
    if(!slots)
        return HJ_MAINBOARD_ERR_INVALID_ARG;

#ifdef _WIN32
    char buf[128];
    if(hj_get_wmi_property("Win32_SystemSlot", "SlotLayout", buf, sizeof(buf))
       == 0)
    {
        *slots = (unsigned int) atoi(buf);
        return HJ_MAINBOARD_OK;
    }
    return HJ_MAINBOARD_ERR_READ_INFO_FAILED;
#else
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
#endif
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_manufacturer_name(char  *buf,
                                                                   size_t size)
{
    return hj_mainboard_vendor(buf, size);
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_product_name(char  *buf,
                                                              size_t size)
{
    return hj_mainboard_model(buf, size);
}

HJ_MAINBOARD_API hj_mainboard_err_t hj_mainboard_version(uint8_t *major,
                                                         uint8_t *minor,
                                                         uint8_t *patch)
{
    if(!major || !minor || !patch)
        return HJ_MAINBOARD_ERR_INVALID_ARG;

    char buf[128] = {0};
    int  ret      = -1;

#ifdef _WIN32
    ret = hj_get_wmi_property("Win32_BaseBoard", "Version", buf, sizeof(buf));
#elif defined(__linux__)
    ret = hj_read_sysfs("/sys/class/dmi/id/board_version", buf, sizeof(buf));
#elif defined(__APPLE__)
    ret = hj_sysctl_string("hw.model", buf, sizeof(buf));
#endif

    if(ret == 0 && buf[0] != '\0')
    {
        if(hj_parse_version_string(buf, major, minor, patch) == 0)
            return HJ_MAINBOARD_OK;
        return HJ_MAINBOARD_ERR_READ_INFO_FAILED;
    }

    *major = *minor = *patch = 0;
    return HJ_MAINBOARD_ERR_NOT_SUPPORTED;
}

#ifdef __cplusplus
}
#endif

#endif // HJ_MAINBOARD_IMPL && !HJ_MAINBOARD_IMPL_DONE