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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

#ifdef _WIN32
inline int _get_wmi_property(const char *wmi_class,
                             const char *prop_name,
                             char       *buf,
                             size_t      size)
{
    HRESULT               hres;
    IWbemLocator         *pLoc        = NULL;
    IWbemServices        *pSvc        = NULL;
    IEnumWbemClassObject *pEnumerator = NULL;
    IWbemClassObject     *pclsObj     = NULL;
    ULONG                 uReturn     = 0;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if(FAILED(hres))
        return -1;

    hres = CoInitializeSecurity(NULL,
                                -1,
                                NULL,
                                NULL,
                                RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                EOAC_NONE,
                                NULL);
    if(FAILED(hres))
    {
        CoUninitialize();
        return -1;
    }

    hres = CoCreateInstance(CLSID_WbemLocator,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (LPVOID *) &pLoc);
    if(FAILED(hres))
    {
        CoUninitialize();
        return -1;
    }

    hres =
        pLoc->ConnectServer(L"ROOT\\CIMV2", NULL, NULL, 0, 0, 0, NULL, &pSvc);
    if(FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    hres = CoSetProxyBlanket(pSvc,
                             RPC_C_AUTHN_WINNT,
                             RPC_C_AUTHZ_NONE,
                             NULL,
                             RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE);
    if(FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    wchar_t query[256];
    swprintf(query, 256, L"SELECT %S FROM %S", prop_name, wmi_class);
    hres =
        pSvc->ExecQuery(L"WQL",
                        query,
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator);
    if(FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
    if(0 == uReturn)
    {
        pEnumerator->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -1;
    }
    VARIANT vtProp;
    VariantInit(&vtProp);
    wchar_t wprop_name[128];
    mbstowcs(wprop_name, prop_name, sizeof(wprop_name) / sizeof(wchar_t) - 1);
    wprop_name[sizeof(wprop_name) / sizeof(wchar_t) - 1] = L'\0';
    hres = pclsObj->Get(wprop_name, 0, &vtProp, 0, 0);
    if(SUCCEEDED(hres) && vtProp.vt == VT_BSTR)
    {
        wcstombs(buf, vtProp.bstrVal, size - 1);
        buf[size - 1] = '\0';
    } else
    {
        buf[0] = '\0';
    }

    VariantClear(&vtProp);
    pclsObj->Release();
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return 0;
}
#endif // _WIN32

#ifdef __linux__
inline int _read_sysfs(const char *path, char *buf, size_t size)
{
    FILE *f = fopen(path, "r");
    if(!f)
        return -1;

    if(!fgets(buf, (int) size, f))
    {
        fclose(f);
        return -1;
    }
    buf[strcspn(buf, "\n")] = 0;
    fclose(f);
    return 0;
}
#endif

#ifdef __APPLE__
inline int _sysctl_string(const char *name, char *buf, size_t size)
{
    size_t len = size;
    if(sysctlbyname(name, buf, &len, NULL, 0) != 0)
        return -1;

    buf[len] = '\0';
    return 0;
}
#endif


// ----------------------------- mainboard API --------------------------------
inline int mainboard_model(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BaseBoard", "Product", buf, size);
#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/board_name", buf, size);
#elif defined(__APPLE__)
    return _sysctl_string("hw.model", buf, size);
#endif
}

inline int mainboard_vendor(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BaseBoard", "Manufacturer", buf, size);
#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/board_vendor", buf, size);
#elif defined(__APPLE__)
    snprintf(buf, size, "Apple Inc.");
    return 0;
#endif
}

inline int mainboard_serial_num(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BaseBoard", "SerialNumber", buf, size);
#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/board_serial", buf, size);
#elif defined(__APPLE__)
    io_registry_entry_t entry =
        IORegistryEntryFromPath(kIOMainPortDefault, "IOService:/");
    if(!entry)
        return -1;
    CFStringRef serial = (CFStringRef) IORegistryEntryCreateCFProperty(
        entry,
        CFSTR("IOPlatformSerialNumber"),
        kCFAllocatorDefault,
        0);
    IOObjectRelease(entry);
    if(!serial)
        return -1;
    CFStringGetCString(serial, buf, size, kCFStringEncodingUTF8);
    CFRelease(serial);
    return 0;
#endif
}

inline int mainboard_bios_version(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BIOS", "SMBIOSBIOSVersion", buf, size);
#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/bios_version", buf, size);
#elif defined(__APPLE__)
    return _sysctl_string("machdep.cpu.brand_string", buf, size);
#endif
}

inline int mainboard_chipset(char *buf, size_t size)
{
    int ret = -1;
#ifdef _WIN32
    ret = _get_wmi_property("Win32_Chipset", "Name", buf, size);
#elif defined(__linux__)
    ret = _read_sysfs("/sys/class/dmi/id/product_name", buf, size);
#elif defined(__APPLE__)
    snprintf(buf, size, "Apple SoC/Chipset");
    ret = 0;
#endif

    if(ret != 0 || buf[0] == '\0')
    {
        strncpy(buf, "Unknown", size - 1);
        buf[size - 1] = '\0';
        return 0;
    }
    return 0;
}

inline int mainboard_memory_slots()
{
#ifdef _WIN32
    char buf[128];
    if(_get_wmi_property("Win32_PhysicalMemoryArray",
                         "MemoryDevices",
                         buf,
                         sizeof(buf))
       == 0)
        return atoi(buf);

    return -1;
#elif defined(__linux__)
    char path[128];
    int  count = 0;
    for(int i = 0; i < 16; i++)
    {
        snprintf(path, sizeof(path), "/sys/class/dmi/id/mem%i", i);
        if(access(path, F_OK) == 0)
            count++;
    }
    return count;
#elif defined(__APPLE__)
    int    slots = 0;
    size_t len   = sizeof(slots);
    if(sysctlbyname("hw.memslots", &slots, &len, NULL, 0) == 0)
        return slots;

    return -1;
#endif
}

inline int mainboard_expansion_slots()
{
#ifdef _WIN32
    char buf[128];
    if(_get_wmi_property("Win32_SystemSlot", "SlotLayout", buf, sizeof(buf))
       == 0)
        return atoi(buf);

    return -1;

#elif defined(__linux__)
    return -1;

#elif defined(__APPLE__)
    return -1;

#endif
}

inline int mainboard_manufacturer_name(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BaseBoard", "Manufacturer", buf, size);

#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/board_vendor", buf, size);

#elif defined(__APPLE__)
    snprintf(buf, size, "Apple Inc.");
    return 0;

#else
    if(size > 0)
        buf[0] = 0;

    return -1;

#endif
}

inline int mainboard_product_name(char *buf, size_t size)
{
#ifdef _WIN32
    return _get_wmi_property("Win32_BaseBoard", "Product", buf, size);

#elif defined(__linux__)
    return _read_sysfs("/sys/class/dmi/id/board_name", buf, size);

#elif defined(__APPLE__)
    return _sysctl_string("hw.model", buf, size);

#else
    if(size > 0)
        buf[0] = 0;

    return -1;

#endif
}

inline int mainboard_version(uint8_t *major, uint8_t *minor, uint8_t *patch)
{
#ifdef _WIN32
    char buf[64] = {0};
    if(_get_wmi_property("Win32_BaseBoard", "Version", buf, sizeof(buf)) != 0)
        return -1;

    int m = 0, n = 0, p = 0;
    if(sscanf(buf, "%hhu.%hhu.%hhu", major, minor, patch) == 3)
        return 0;

    *major = *minor = *patch = 0;
    return 0;

#elif defined(__linux__)
    FILE *fp = fopen("/sys/class/dmi/id/board_version", "r");
    if(!fp)
        return -1;

    char buf[32] = {0};
    if(!fgets(buf, sizeof(buf), fp))
    {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    int m = 0, n = 0, p = 0;
    if(sscanf(buf, "%hhu.%hhu.%hhu", major, minor, patch) == 3)
        return 0;

    *major = *minor = *patch = 0;
    return 0;

#elif defined(__APPLE__)
    char buf[64] = {0};
    if(_sysctl_string("hw.model", buf, sizeof(buf)) != 0)
        return -1;

    *major = *minor = *patch = 0;
    return 0;

#else
    *major = *minor = *patch = 0;
    return -1;

#endif
}

#ifdef __cplusplus
}
#endif

#endif // MAINBOARD_H
