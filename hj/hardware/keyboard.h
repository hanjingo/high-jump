#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#ifdef __linux__
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <dirent.h>

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <hidclass.h>
#include <dbt.h>
#pragma comment(lib, "setupapi.lib")

#elif defined(__APPLE__)
#ifdef __cplusplus
#include <IOKit/hid/IOHIDManager.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char device_path[256];
    char manufacturer[128];
    char product[128];
    char serial[128];
} keyboard_info_t;

typedef struct
{
    int keycode;
    int pressed;
} key_event_t;

inline int keyboard_enumerate(keyboard_info_t *infos, int max_count)
{
    int count = 0;

#ifdef __linux__
    DIR *dir = opendir("/dev/input");
    if(!dir)
        return 0;
    struct dirent *entry;
    while((entry = readdir(dir)) && count < max_count)
    {
        if(strncmp(entry->d_name, "event", 5) != 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY);
        if(fd < 0)
            continue;

        struct input_id id;
        ioctl(fd, EVIOCGID, &id);
        strncpy(infos[count].device_path,
                path,
                sizeof(infos[count].device_path));
        snprintf(infos[count].manufacturer,
                 sizeof(infos[count].manufacturer),
                 "vendor_%04x",
                 id.vendor);
        snprintf(infos[count].product,
                 sizeof(infos[count].product),
                 "product_%04x",
                 id.product);
        snprintf(infos[count].serial,
                 sizeof(infos[count].serial),
                 "version_%04x",
                 id.version);
        close(fd);
        count++;
    }
    closedir(dir);

#elif defined(_WIN32) || defined(_WIN64)
    HDEVINFO devs = SetupDiGetClassDevs(&GUID_DEVCLASS_KEYBOARD,
                                        NULL,
                                        NULL,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(devs == INVALID_HANDLE_VALUE)
        return 0;

    SP_DEVINFO_DATA devInfo;
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    for(DWORD i = 0;
        SetupDiEnumDeviceInfo(devs, i, &devInfo) && count < max_count;
        ++i)
    {
        char buf[256] = {0};
        SetupDiGetDeviceRegistryPropertyA(devs,
                                          &devInfo,
                                          SPDRP_FRIENDLYNAME,
                                          NULL,
                                          (PBYTE) buf,
                                          sizeof(buf),
                                          NULL);
        strncpy(infos[count].product, buf, sizeof(infos[count].product));
        SetupDiGetDeviceRegistryPropertyA(devs,
                                          &devInfo,
                                          SPDRP_MFG,
                                          NULL,
                                          (PBYTE) buf,
                                          sizeof(buf),
                                          NULL);
        strncpy(infos[count].manufacturer,
                buf,
                sizeof(infos[count].manufacturer));
        infos[count].device_path[0] = 0;
        infos[count].serial[0]      = 0;
        count++;
    }
    SetupDiDestroyDeviceInfoList(devs);

#elif defined(__APPLE__)
#ifdef __cplusplus
    IOHIDManagerRef manager =
        IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!manager)
        return 0;

    CFMutableDictionaryRef match =
        CFDictionaryCreateMutable(kCFAllocatorDefault,
                                  0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);
    int usagePage = 0x01, usage = 0x06;
    CFDictionarySetValue(match,
                         CFSTR(kIOHIDDeviceUsagePageKey),
                         CFNumberCreate(NULL, kCFNumberIntType, &usagePage));
    CFDictionarySetValue(match,
                         CFSTR(kIOHIDDeviceUsageKey),
                         CFNumberCreate(NULL, kCFNumberIntType, &usage));
    IOHIDManagerSetDeviceMatching(manager, match);
    CFRelease(match);

    CFSetRef device_set = IOHIDManagerCopyDevices(manager);
    if(device_set)
    {
        CFIndex        num = CFSetGetCount(device_set);
        IOHIDDeviceRef devices[num];
        CFSetGetValues(device_set, (const void **) devices);
        for(CFIndex i = 0; i < num && count < max_count; ++i)
        {
            CFTypeRef manuRef =
                IOHIDDeviceGetProperty(devices[i],
                                       CFSTR(kIOHIDManufacturerKey));
            CFTypeRef prodRef =
                IOHIDDeviceGetProperty(devices[i], CFSTR(kIOHIDProductKey));
            CFTypeRef serRef =
                IOHIDDeviceGetProperty(devices[i],
                                       CFSTR(kIOHIDSerialNumberKey));
            infos[count].device_path[0] = 0;
            if(manuRef && CFGetTypeID(manuRef) == CFStringGetTypeID())
                CFStringGetCString((CFStringRef) manuRef,
                                   infos[count].manufacturer,
                                   sizeof(infos[count].manufacturer),
                                   kCFStringEncodingUTF8);
            else
                infos[count].manufacturer[0] = 0;

            if(prodRef && CFGetTypeID(prodRef) == CFStringGetTypeID())
                CFStringGetCString((CFStringRef) prodRef,
                                   infos[count].product,
                                   sizeof(infos[count].product),
                                   kCFStringEncodingUTF8);
            else
                infos[count].product[0] = 0;

            if(serRef && CFGetTypeID(serRef) == CFStringGetTypeID())
                CFStringGetCString((CFStringRef) serRef,
                                   infos[count].serial,
                                   sizeof(infos[count].serial),
                                   kCFStringEncodingUTF8);
            else
                infos[count].serial[0] = 0;

            count++;
        }
        CFRelease(device_set);
    }
    CFRelease(manager);
#endif

#endif

    return count;
}

inline int keyboard_open(const char *device_path)
{
#ifdef __linux__
    return open(device_path, O_RDONLY | O_NONBLOCK);

#elif defined(_WIN32) || defined(_WIN64)
    (void) device_path;
    return 1;

#elif defined(__APPLE__)
    (void) device_path;
    return 1;

#endif
}

inline void keyboard_close(int handle)
{
#ifdef __linux__
    if(handle >= 0)
        close(handle);

#elif defined(_WIN32) || defined(_WIN64)
    (void) handle;

#elif defined(__APPLE__)
    (void) handle;

#endif
}

inline int keyboard_read_event(int handle, key_event_t *event)
{
#ifdef __linux__
    struct input_event ev;
    ssize_t            n = read(handle, &ev, sizeof(ev));
    if(n == sizeof(ev) && ev.type == EV_KEY)
    {
        event->keycode = ev.code;
        event->pressed = ev.value;
        return 0;
    }
    return -1;

#elif defined(_WIN32) || defined(_WIN64)
    for(int vk = 0x08; vk <= 0xFE; ++vk)
    {
        SHORT state = GetAsyncKeyState(vk);
        if(state & 0x8000)
        {
            event->keycode = vk;
            event->pressed = 1;
            return 0;
        }
    }
    return -1;

#elif defined(__APPLE__)
    for(int key = 0; key < 128; ++key)
    {
        if(CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, key))
        {
            event->keycode = key;
            event->pressed = 1;
            return 0;
        }
    }
    return -1;

#endif
}

inline int keyboard_set_repeat(int handle, int delay_ms, int rate_ms)
{
#ifdef __linux__
    int rep[2] = {delay_ms, rate_ms};
    return ioctl(handle, EVIOCSREP, rep);

#elif defined(_WIN32) || defined(_WIN64)
    (void) handle;
    return SystemParametersInfoA(SPI_SETKEYBOARDSPEED, rate_ms, NULL, 0)
                   && SystemParametersInfoA(SPI_SETKEYBOARDDELAY,
                                            delay_ms,
                                            NULL,
                                            0)
               ? 0
               : -1;

#elif defined(__APPLE__)
    (void) handle;
    (void) delay_ms;
    (void) rate_ms;
    return -1;

#endif

    return -1;
}

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_H