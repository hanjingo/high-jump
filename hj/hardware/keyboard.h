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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_KEYBOARD_API
#if defined(HJ_KEYBOARD_STATIC)
#define HJ_KEYBOARD_API static inline
#else
#define HJ_KEYBOARD_API extern
#endif
#endif

typedef enum
{
    HJ_KEYBOARD_OK                = 0,
    HJ_KEYBOARD_ERR_UNKNOWN       = -1,
    HJ_KEYBOARD_ERR_NOT_SUPPORTED = -2,
    HJ_KEYBOARD_ERR_INVALID_ARG   = -3,
    HJ_KEYBOARD_ERR_INTERNAL      = -4,
    HJ_KEYBOARD_ERR_OPEN_FAILED   = -5,
    HJ_KEYBOARD_ERR_READ_FAILED   = -6,
    HJ_KEYBOARD_ERR_NO_DATA       = -7
} hj_keyboard_err_t;

typedef enum
{
    HJ_KEY_STATE_RELEASED = 0,
    HJ_KEY_STATE_PRESSED  = 1,
    HJ_KEY_STATE_REPEAT   = 2
} hj_key_state_t;

typedef struct
{
    char device_path[256];
    char manufacturer[128];
    char product[128];
    char serial[128];
} hj_keyboard_info_t;

typedef struct
{
    int            keycode;
    hj_key_state_t state;
} hj_key_event_t;

typedef intptr_t hj_keyboard_handle_t;

#define HJ_INVALID_HANDLE ((hj_keyboard_handle_t) - 1)

// ------------------------ Keyboard API Declarations ------------------------

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_enumerate(hj_keyboard_info_t *infos, int max_count, int *out_count);

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_open(const char *device_path, hj_keyboard_handle_t *out_handle);

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_close(hj_keyboard_handle_t handle);

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_read_event(hj_keyboard_handle_t handle, hj_key_event_t *event);

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_set_repeat(hj_keyboard_handle_t handle, int delay_ms, int rate_ms);

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_H


// --------------------- Implementation -------------------------
#if (defined(HJ_KEYBOARD_IMPL) || defined(HJ_KEYBOARD_STATIC))                 \
    && !defined(HJ_KEYBOARD_IMPL_DONE)
#define HJ_KEYBOARD_IMPL_DONE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <dirent.h>

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>
#include <hidclass.h>
#pragma comment(lib, "setupapi.lib")

// Define GUID_DEVINTERFACE_KEYBOARD if not present
DEFINE_GUID(GUID_DEVINTERFACE_KEYBOARD,
            0x88467AB4,
            0x0408,
            0x11D1,
            0xA3,
            0x5C,
            0x00,
            0xA0,
            0xC9,
            0x22,
            0x31,
            0x96);

#elif defined(__APPLE__)
#include <IOKit/hid/IOHIDManager.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

HJ_KEYBOARD_API void
hj_keyboard_safe_strcpy(char *dest, const char *src, size_t dest_size)
{
    if(!dest || dest_size == 0)
        return;
    if(!src)
    {
        dest[0] = '\0';
        return;
    }
    snprintf(dest, dest_size, "%s", src);
}

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_enumerate(hj_keyboard_info_t *infos, int max_count, int *out_count)
{
    if(!out_count)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

    *out_count = 0;
    if(!infos || max_count <= 0)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

    int count = 0;

#if defined(__linux__)
    DIR *dir = opendir("/dev/input");
    if(!dir)
        return HJ_KEYBOARD_OK;

    struct dirent *entry;
    while((entry = readdir(dir)) && count < max_count)
    {
        if(strncmp(entry->d_name, "event", 5) != 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if(fd < 0)
            continue;

        // Verify if device supports keyboard events
        unsigned long evbit = 0;
        if(ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit) < 0
           || !(evbit & (1 << EV_KEY)))
        {
            close(fd);
            continue;
        }

        struct input_id id;
        memset(&infos[count], 0, sizeof(hj_keyboard_info_t));
        ioctl(fd, EVIOCGID, &id);

        hj_keyboard_safe_strcpy(infos[count].device_path,
                                path,
                                sizeof(infos[count].device_path));

        char name[128] = "Unknown Keyboard";
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        hj_keyboard_safe_strcpy(infos[count].product,
                                name,
                                sizeof(infos[count].product));

        snprintf(infos[count].manufacturer,
                 sizeof(infos[count].manufacturer),
                 "Vendor_0x%04x",
                 id.vendor);
        snprintf(infos[count].serial,
                 sizeof(infos[count].serial),
                 "Bus_0x%04x",
                 id.bustype);

        close(fd);
        count++;
    }
    closedir(dir);

#elif defined(_WIN32) || defined(_WIN64)
    HDEVINFO devs = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_KEYBOARD,
                                         NULL,
                                         NULL,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(devs == INVALID_HANDLE_VALUE)
        return HJ_KEYBOARD_OK;

    SP_DEVICE_INTERFACE_DATA ifData;
    ifData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for(DWORD i = 0; SetupDiEnumDeviceInterfaces(devs,
                                                 NULL,
                                                 &GUID_DEVINTERFACE_KEYBOARD,
                                                 i,
                                                 &ifData)
                     && count < max_count;
        ++i)
    {
        DWORD reqSize = 0;
        SetupDiGetDeviceInterfaceDetailA(devs,
                                         &ifData,
                                         NULL,
                                         0,
                                         &reqSize,
                                         NULL);
        if(reqSize == 0)
            continue;

        PSP_DEVICE_INTERFACE_DETAIL_DATA_A detail =
            (PSP_DEVICE_INTERFACE_DETAIL_DATA_A) malloc(reqSize);
        if(!detail)
            continue;

        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
        SP_DEVINFO_DATA devData;
        devData.cbSize = sizeof(SP_DEVINFO_DATA);

        if(SetupDiGetDeviceInterfaceDetailA(devs,
                                            &ifData,
                                            detail,
                                            reqSize,
                                            NULL,
                                            &devData))
        {
            memset(&infos[count], 0, sizeof(hj_keyboard_info_t));
            hj_keyboard_safe_strcpy(infos[count].device_path,
                                    detail->DevicePath,
                                    sizeof(infos[count].device_path));

            char buf[256] = {0};
            if(SetupDiGetDeviceRegistryPropertyA(devs,
                                                 &devData,
                                                 SPDRP_FRIENDLYNAME,
                                                 NULL,
                                                 (PBYTE) buf,
                                                 sizeof(buf),
                                                 NULL)
               || SetupDiGetDeviceRegistryPropertyA(devs,
                                                    &devData,
                                                    SPDRP_DEVICEDESC,
                                                    NULL,
                                                    (PBYTE) buf,
                                                    sizeof(buf),
                                                    NULL))
            {
                hj_keyboard_safe_strcpy(infos[count].product,
                                        buf,
                                        sizeof(infos[count].product));
            }

            if(SetupDiGetDeviceRegistryPropertyA(devs,
                                                 &devData,
                                                 SPDRP_MFG,
                                                 NULL,
                                                 (PBYTE) buf,
                                                 sizeof(buf),
                                                 NULL))
            {
                hj_keyboard_safe_strcpy(infos[count].manufacturer,
                                        buf,
                                        sizeof(infos[count].manufacturer));
            }

            count++;
        }
        free(detail);
    }
    SetupDiDestroyDeviceInfoList(devs);

#elif defined(__APPLE__)
    IOHIDManagerRef manager =
        IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!manager)
        return HJ_KEYBOARD_OK;

    CFMutableDictionaryRef match =
        CFDictionaryCreateMutable(kCFAllocatorDefault,
                                  0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);
    int         usagePage = 0x01, usage = 0x06; // Generic Desktop Keyboard
    CFNumberRef pageNum =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
    CFNumberRef usageNum =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);

    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsagePageKey), pageNum);
    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsageKey), usageNum);
    CFRelease(pageNum);
    CFRelease(usageNum);

    IOHIDManagerSetDeviceMatching(manager, match);
    CFRelease(match);

    CFSetRef device_set = IOHIDManagerCopyDevices(manager);
    if(device_set)
    {
        CFIndex num = CFSetGetCount(device_set);
        if(num > 0)
        {
            IOHIDDeviceRef *devices =
                (IOHIDDeviceRef *) malloc(sizeof(IOHIDDeviceRef) * num);
            if(devices)
            {
                CFSetGetValues(device_set, (const void **) devices);
                for(CFIndex i = 0; i < num && count < max_count; ++i)
                {
                    memset(&infos[count], 0, sizeof(hj_keyboard_info_t));

                    // Use unique address as identifier string
                    snprintf(infos[count].device_path,
                             sizeof(infos[count].device_path),
                             "iohid://%p",
                             (void *) devices[i]);

                    CFTypeRef manuRef =
                        IOHIDDeviceGetProperty(devices[i],
                                               CFSTR(kIOHIDManufacturerKey));
                    CFTypeRef prodRef =
                        IOHIDDeviceGetProperty(devices[i],
                                               CFSTR(kIOHIDProductKey));
                    CFTypeRef serRef =
                        IOHIDDeviceGetProperty(devices[i],
                                               CFSTR(kIOHIDSerialNumberKey));

                    if(manuRef && CFGetTypeID(manuRef) == CFStringGetTypeID())
                        CFStringGetCString((CFStringRef) manuRef,
                                           infos[count].manufacturer,
                                           sizeof(infos[count].manufacturer),
                                           kCFStringEncodingUTF8);

                    if(prodRef && CFGetTypeID(prodRef) == CFStringGetTypeID())
                        CFStringGetCString((CFStringRef) prodRef,
                                           infos[count].product,
                                           sizeof(infos[count].product),
                                           kCFStringEncodingUTF8);

                    if(serRef && CFGetTypeID(serRef) == CFStringGetTypeID())
                        CFStringGetCString((CFStringRef) serRef,
                                           infos[count].serial,
                                           sizeof(infos[count].serial),
                                           kCFStringEncodingUTF8);

                    count++;
                }
                free(devices);
            }
        }
        CFRelease(device_set);
    }
    CFRelease(manager);
#endif

    *out_count = count;
    return HJ_KEYBOARD_OK;
}

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_open(const char *device_path, hj_keyboard_handle_t *out_handle)
{
    if(!out_handle)
        return HJ_KEYBOARD_ERR_INVALID_ARG;
    *out_handle = HJ_INVALID_HANDLE;

    if(!device_path || strlen(device_path) == 0)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

#if defined(__linux__)
    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if(fd < 0)
        return HJ_KEYBOARD_ERR_OPEN_FAILED;

    *out_handle = (hj_keyboard_handle_t) fd;
    return HJ_KEYBOARD_OK;

#elif defined(_WIN32) || defined(_WIN64)
    HANDLE hDev = CreateFileA(device_path,
                              0,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if(hDev == INVALID_HANDLE_VALUE)
        return HJ_KEYBOARD_ERR_OPEN_FAILED;

    *out_handle = (hj_keyboard_handle_t) hDev;
    return HJ_KEYBOARD_OK;

#elif defined(__APPLE__)
    IOHIDManagerRef manager =
        IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!manager)
        return HJ_KEYBOARD_ERR_OPEN_FAILED;

    IOHIDManagerSetDeviceMatching(manager, NULL);
    CFSetRef device_set = IOHIDManagerCopyDevices(manager);

    IOHIDDeviceRef target_device = NULL;
    if(device_set)
    {
        CFIndex num = CFSetGetCount(device_set);
        if(num > 0)
        {
            IOHIDDeviceRef *devices =
                (IOHIDDeviceRef *) malloc(sizeof(IOHIDDeviceRef) * num);
            if(devices)
            {
                CFSetGetValues(device_set, (const void **) devices);
                for(CFIndex i = 0; i < num; ++i)
                {
                    char current_path[256];
                    snprintf(current_path,
                             sizeof(current_path),
                             "iohid://%p",
                             (void *) devices[i]);
                    if(strcmp(current_path, device_path) == 0)
                    {
                        target_device = devices[i];
                        break;
                    }
                }
                free(devices);
            }
        }
        CFRelease(device_set);
    }

    if(!target_device)
    {
        CFRelease(manager);
        return HJ_KEYBOARD_ERR_OPEN_FAILED;
    }

    IOReturn res = IOHIDDeviceOpen(target_device, kIOHIDOptionsTypeNone);
    if(res != kIOReturnSuccess)
    {
        CFRelease(manager);
        return HJ_KEYBOARD_ERR_OPEN_FAILED;
    }

    hj_mac_keyboard_t *dev_ctx =
        (hj_mac_keyboard_t *) calloc(1, sizeof(hj_mac_keyboard_t));
    if(!dev_ctx)
    {
        CFRelease(manager);
        return HJ_KEYBOARD_ERR_INTERNAL;
    }
    dev_ctx->device = target_device;
    IOHIDDeviceRegisterInputValueCallback(target_device,
                                          mac_keyboard_callback,
                                          dev_ctx);
    IOHIDDeviceScheduleWithRunLoop(target_device,
                                   CFRunLoopGetCurrent(),
                                   kCFRunLoopDefaultMode);

    CFRelease(manager);
    *out_handle = (hj_keyboard_handle_t) dev_ctx;
    return HJ_KEYBOARD_OK;

#else
    return HJ_KEYBOARD_ERR_NOT_SUPPORTED;

#endif
}

HJ_KEYBOARD_API hj_keyboard_err_t hj_keyboard_close(hj_keyboard_handle_t handle)
{
    if(handle == HJ_INVALID_HANDLE)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

#if defined(__linux__)
    close((int) handle);

#elif defined(_WIN32) || defined(_WIN64)
    CloseHandle((HANDLE) handle);

#elif defined(__APPLE__)
    hj_mac_keyboard_t *dev_ctx = (hj_mac_keyboard_t *) handle;
    if(dev_ctx)
    {
        if(dev_ctx->device)
        {
            IOHIDDeviceUnscheduleFromRunLoop(dev_ctx->device,
                                             CFRunLoopGetCurrent(),
                                             kCFRunLoopDefaultMode);
            IOHIDDeviceClose(dev_ctx->device, kIOHIDOptionsTypeNone);
        }
        free(dev_ctx);
    }

#endif
    return HJ_KEYBOARD_OK;
}

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_read_event(hj_keyboard_handle_t handle, hj_key_event_t *event)
{
    if(!event || handle == HJ_INVALID_HANDLE)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

#if defined(__linux__)
    struct input_event ev;
    ssize_t            n = read((int) handle, &ev, sizeof(ev));
    if(n == sizeof(ev))
    {
        if(ev.type == EV_KEY)
        {
            event->keycode = ev.code;
            if(ev.value == 0)
                event->state = HJ_KEY_STATE_RELEASED;
            else if(ev.value == 1)
                event->state = HJ_KEY_STATE_PRESSED;
            else if(ev.value == 2)
                event->state = HJ_KEY_STATE_REPEAT;
            return HJ_KEYBOARD_OK;
        }
        return HJ_KEYBOARD_ERR_NO_DATA;
    }
    return HJ_KEYBOARD_ERR_NO_DATA;

#elif defined(_WIN32) || defined(_WIN64)
    DWORD   bytesRead  = 0;
    uint8_t buffer[64] = {0};
    if(ReadFile((HANDLE) handle, buffer, sizeof(buffer), &bytesRead, NULL)
       && bytesRead > 0)
    {
        event->keycode = buffer[0];
        event->state =
            (buffer[1] != 0) ? HJ_KEY_STATE_PRESSED : HJ_KEY_STATE_RELEASED;
        return HJ_KEYBOARD_OK;
    }
    return HJ_KEYBOARD_ERR_NO_DATA;

#elif defined(__APPLE__)
    hj_mac_keyboard_t *dev_ctx = (hj_mac_keyboard_t *) handle;
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    if(dev_ctx->has_event)
    {
        *event             = dev_ctx->last_event;
        dev_ctx->has_event = 0;
        return HJ_KEYBOARD_OK;
    }
    return HJ_KEYBOARD_ERR_NO_DATA;

#else
    return HJ_KEYBOARD_ERR_NOT_SUPPORTED;

#endif
}

HJ_KEYBOARD_API hj_keyboard_err_t
hj_keyboard_set_repeat(hj_keyboard_handle_t handle, int delay_ms, int rate_ms)
{
    if(handle == HJ_INVALID_HANDLE || delay_ms < 0 || rate_ms <= 0)
        return HJ_KEYBOARD_ERR_INVALID_ARG;

#if defined(__linux__)
    int rep[2] = {delay_ms, rate_ms};
    if(ioctl(handle, EVIOCSREP, rep) == 0)
        return HJ_KEYBOARD_OK;

    return HJ_KEYBOARD_ERR_INTERNAL;

#elif defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
    // Explicitly refuse to modify global OS-wide keyboard parameters
    (void) handle;
    return HJ_KEYBOARD_ERR_NOT_SUPPORTED;

#else
    return HJ_KEYBOARD_ERR_NOT_SUPPORTED;
#endif
}

#ifdef __cplusplus
}
#endif

#endif // HJ_KEYBOARD_IMPL && !HJ_KEYBOARD_IMPL_DONE