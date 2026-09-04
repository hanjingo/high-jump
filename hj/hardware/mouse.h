/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_MOUSE_API
#if defined(HJ_MOUSE_STATIC)
#define HJ_MOUSE_API static inline
#else
#define HJ_MOUSE_API extern
#endif
#endif

typedef enum
{
    HJ_MOUSE_SUCCESS             = 0,
    HJ_MOUSE_ERROR_INVALID_PARAM = -1,
    HJ_MOUSE_ERROR_IO            = -2,
    HJ_MOUSE_ERROR_NO_DATA       = -3,
    HJ_MOUSE_ERROR_NOT_SUPPORTED = -4
} hj_mouse_err_t;

typedef enum
{
    HJ_MOUSE_BTN_NONE   = 0,
    HJ_MOUSE_BTN_LEFT   = 1,
    HJ_MOUSE_BTN_RIGHT  = 2,
    HJ_MOUSE_BTN_MIDDLE = 3,
    HJ_MOUSE_BTN_EXTRA1 = 4,
    HJ_MOUSE_BTN_EXTRA2 = 5
} hj_mouse_button_t;

typedef enum
{
    HJ_MOUSE_EVENT_NONE   = 0,
    HJ_MOUSE_EVENT_MOTION = 1,
    HJ_MOUSE_EVENT_BUTTON = 2,
    HJ_MOUSE_EVENT_WHEEL  = 3
} hj_mouse_event_type_t;

typedef struct
{
    char     device_path[256];
    char     manufacturer[128];
    char     product[128];
    char     serial[128];
    uint16_t vendor_id;
    uint16_t product_id;
} hj_mouse_info_t;

typedef struct
{
    hj_mouse_event_type_t type;
    int32_t               dx;
    int32_t               dy;
    int32_t               wheel_delta;
    hj_mouse_button_t     button;
    int                   pressed;
    uint64_t              timestamp_us;
} hj_mouse_event_t;

// ------------------------ API Define ------------------------
HJ_MOUSE_API hj_mouse_err_t hj_mouse_enumerate(hj_mouse_info_t *infos,
                                               int              max_count,
                                               int             *out);

HJ_MOUSE_API intptr_t hj_mouse_open(const char *device_path);

HJ_MOUSE_API void hj_mouse_close(intptr_t handle);

HJ_MOUSE_API hj_mouse_err_t hj_mouse_read_event(intptr_t          handle,
                                                hj_mouse_event_t *event);

HJ_MOUSE_API hj_mouse_err_t hj_mouse_set_param(intptr_t handle, int accel);

#ifdef __cplusplus
}
#endif

#endif // MOUSE_H


// --------------------- Implement -------------------------
#if (defined(HJ_MOUSE_IMPL) || defined(HJ_MOUSE_STATIC))                       \
    && !defined(HJ_MOUSE_IMPL_DONE)
#define HJ_MOUSE_IMPL_DONE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/time.h>

#elif defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <process.h>

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

HJ_MOUSE_API uint64_t hj_mouse_get_timestamp_us(void)
{
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (uint64_t) ((counter.QuadPart * 1000000) / freq.QuadPart);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

HJ_MOUSE_API void hj_safe_strcpy(char *dest, const char *src, size_t dest_size)
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

// ---------------- Linux Implement ----------------
#ifdef __linux__

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x) ((x) % BITS_PER_LONG)
#define BIT(x) (1UL << OFF(x))
#define LONG(x) ((x) / BITS_PER_LONG)
#define IS_BIT_SET(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

HJ_MOUSE_API
hj_mouse_err_t hj_mouse_parse_input_event(const struct input_event *ev,
                                          hj_mouse_event_t         *event)
{
    if(!ev || !event)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    memset(event, 0, sizeof(hj_mouse_event_t));
    event->timestamp_us =
        (uint64_t) ev->time.tv_sec * 1000000 + ev->time.tv_usec;

    if(ev->type == EV_REL)
    {
        if(ev->code == REL_X)
        {
            event->type = HJ_MOUSE_EVENT_MOTION;
            event->dx   = ev->value;
            return HJ_MOUSE_SUCCESS;
        } else if(ev->code == REL_Y)
        {
            event->type = HJ_MOUSE_EVENT_MOTION;
            event->dy   = ev->value;
            return HJ_MOUSE_SUCCESS;
        } else if(ev->code == REL_WHEEL)
        {
            event->type        = HJ_MOUSE_EVENT_WHEEL;
            event->wheel_delta = ev->value;
            return HJ_MOUSE_SUCCESS;
        }
    } else if(ev->type == EV_KEY)
    {
        if(ev->code >= BTN_LEFT && ev->code <= BTN_TASK)
        {
            event->type    = HJ_MOUSE_EVENT_BUTTON;
            event->pressed = ev->value;
            switch(ev->code)
            {
                case BTN_LEFT:
                    event->button = HJ_MOUSE_BTN_LEFT;
                    break;
                case BTN_RIGHT:
                    event->button = HJ_MOUSE_BTN_RIGHT;
                    break;
                case BTN_MIDDLE:
                    event->button = HJ_MOUSE_BTN_MIDDLE;
                    break;
                case BTN_SIDE:
                    event->button = HJ_MOUSE_BTN_EXTRA1;
                    break;
                case BTN_EXTRA:
                    event->button = HJ_MOUSE_BTN_EXTRA2;
                    break;
                default:
                    event->button = HJ_MOUSE_BTN_NONE;
                    break;
            }
            return HJ_MOUSE_SUCCESS;
        }
    }

    return HJ_MOUSE_ERROR_NO_DATA;
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_enumerate(hj_mouse_info_t *infos,
                                               int              max_count,
                                               int             *out)
{
    if(!infos || max_count <= 0 || !out)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    *out     = 0;
    DIR *dir = opendir("/dev/input");
    if(!dir)
        return HJ_MOUSE_ERROR_IO;

    int            count = 0;
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL && count < max_count)
    {
        if(strncmp(entry->d_name, "event", 5) != 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if(fd < 0)
            continue;

        unsigned long evbit[NBITS(EV_MAX)]   = {0};
        unsigned long keybit[NBITS(KEY_MAX)] = {0};
        unsigned long relbit[NBITS(REL_MAX)] = {0};
        if(ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0
           || ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0
           || ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) < 0)
        {
            close(fd);
            continue;
        }

        if(IS_BIT_SET(EV_REL, evbit) && IS_BIT_SET(REL_X, relbit)
           && (IS_BIT_SET(BTN_MOUSE, keybit) || IS_BIT_SET(BTN_LEFT, keybit)))
        {
            memset(&infos[count], 0, sizeof(hj_mouse_info_t));
            hj_safe_strcpy(infos[count].device_path,
                           path,
                           sizeof(infos[count].device_path));

            char name[128] = "Unknown Device";
            if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) >= 0)
            {
                hj_safe_strcpy(infos[count].product,
                               name,
                               sizeof(infos[count].product));
            }

            struct input_id id;
            if(ioctl(fd, EVIOCGID, &id) >= 0)
            {
                infos[count].vendor_id  = id.vendor;
                infos[count].product_id = id.product;
                snprintf(infos[count].manufacturer,
                         sizeof(infos[count].manufacturer),
                         "VID_%04x",
                         id.vendor);
                snprintf(infos[count].serial,
                         sizeof(infos[count].serial),
                         "%04x",
                         id.version);
            }

            count++;
        }
        close(fd);
    }
    closedir(dir);
    *out = count;
    return HJ_MOUSE_SUCCESS;
}

HJ_MOUSE_API intptr_t hj_mouse_open(const char *device_path)
{
    if(!device_path)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    return (fd < 0) ? (intptr_t) HJ_MOUSE_ERROR_IO : (intptr_t) fd;
}

HJ_MOUSE_API void hj_mouse_close(intptr_t handle)
{
    if(handle >= 0)
        close((int) handle);
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_read_event(intptr_t          handle,
                                                hj_mouse_event_t *event)
{
    if(handle < 0 || !event)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    struct input_event ev;
    ssize_t            n = read((int) handle, &ev, sizeof(ev));
    if(n != sizeof(ev))
        return HJ_MOUSE_ERROR_NO_DATA;

    return hj_mouse_parse_input_event(&ev, event);
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_set_param(intptr_t handle, int accel)
{
    (void) handle;
    (void) accel;
    return HJ_MOUSE_ERROR_NOT_SUPPORTED;
}

// ---------------- Windows RawInput ----------------
#elif defined(_WIN32) || defined(_WIN64)

#define HJ_WIN32_EVENT_QUEUE_SIZE 128

typedef struct
{
    hj_mouse_event_t queue[HJ_WIN32_EVENT_QUEUE_SIZE];
    int              head;
    int              tail;
    CRITICAL_SECTION cs;
    HANDLE           thread_handle;
    HWND             hwnd;
    volatile int     running;
    HANDLE           target_raw_device;
    HANDLE           init_event;
} hj_win32_mouse_ctx_t;

HJ_MOUSE_API void hj_win32_push_event(hj_win32_mouse_ctx_t   *ctx,
                                      const hj_mouse_event_t *evt)
{
    EnterCriticalSection(&ctx->cs);
    int next = (ctx->head + 1) % HJ_WIN32_EVENT_QUEUE_SIZE;
    if(next != ctx->tail)
    {
        ctx->queue[ctx->head] = *evt;
        ctx->head             = next;
    }
    LeaveCriticalSection(&ctx->cs);
}

static LRESULT CALLBACK hj_win32_raw_input_proc(HWND   hwnd,
                                                UINT   msg,
                                                WPARAM wParam,
                                                LPARAM lParam)
{
    if(msg == WM_INPUT)
    {
        hj_win32_mouse_ctx_t *ctx =
            (hj_win32_mouse_ctx_t *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if(ctx)
        {
            UINT dwSize = 0;
            GetRawInputData((HRAWINPUT) lParam,
                            RID_INPUT,
                            NULL,
                            &dwSize,
                            sizeof(RAWINPUTHEADER));
            if(dwSize > 0)
            {
                LPBYTE lpb = (LPBYTE) malloc(dwSize);
                if(lpb)
                {
                    if(GetRawInputData((HRAWINPUT) lParam,
                                       RID_INPUT,
                                       lpb,
                                       &dwSize,
                                       sizeof(RAWINPUTHEADER))
                       == dwSize)
                    {
                        RAWINPUT *raw = (RAWINPUT *) lpb;
                        if(raw->header.dwType == RIM_TYPEMOUSE)
                        {
                            if(ctx->target_raw_device == NULL
                               || raw->header.hDevice == ctx->target_raw_device)
                            {
                                hj_mouse_event_t evt;
                                memset(&evt, 0, sizeof(evt));
                                evt.timestamp_us = hj_mouse_get_timestamp_us();

                                if(raw->data.mouse.lLastX != 0
                                   || raw->data.mouse.lLastY != 0)
                                {
                                    evt.type = HJ_MOUSE_EVENT_MOTION;
                                    evt.dx   = raw->data.mouse.lLastX;
                                    evt.dy   = raw->data.mouse.lLastY;
                                    hj_win32_push_event(ctx, &evt);
                                }

                                USHORT flags = raw->data.mouse.usButtonFlags;
                                if(flags & RI_MOUSE_LEFT_BUTTON_DOWN)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_LEFT;
                                    evt.pressed = 1;
                                    hj_win32_push_event(ctx, &evt);
                                } else if(flags & RI_MOUSE_LEFT_BUTTON_UP)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_LEFT;
                                    evt.pressed = 0;
                                    hj_win32_push_event(ctx, &evt);
                                }
                                if(flags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_RIGHT;
                                    evt.pressed = 1;
                                    hj_win32_push_event(ctx, &evt);
                                } else if(flags & RI_MOUSE_RIGHT_BUTTON_UP)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_RIGHT;
                                    evt.pressed = 0;
                                    hj_win32_push_event(ctx, &evt);
                                }
                                if(flags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_MIDDLE;
                                    evt.pressed = 1;
                                    hj_win32_push_event(ctx, &evt);
                                } else if(flags & RI_MOUSE_MIDDLE_BUTTON_UP)
                                {
                                    evt.type    = HJ_MOUSE_EVENT_BUTTON;
                                    evt.button  = HJ_MOUSE_BTN_MIDDLE;
                                    evt.pressed = 0;
                                    hj_win32_push_event(ctx, &evt);
                                }
                                if(flags & RI_MOUSE_WHEEL)
                                {
                                    evt.type = HJ_MOUSE_EVENT_WHEEL;
                                    evt.wheel_delta =
                                        (int16_t) raw->data.mouse.usButtonData;
                                    hj_win32_push_event(ctx, &evt);
                                }
                            }
                        }
                    }
                    free(lpb);
                }
            }
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static unsigned __stdcall hj_win32_raw_input_thread(void *arg)
{
    hj_win32_mouse_ctx_t *ctx = (hj_win32_mouse_ctx_t *) arg;

    WNDCLASSEXA wc   = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc   = hj_win32_raw_input_proc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "HJ_Mouse_RawInput_Class";
    RegisterClassExA(&wc);

    ctx->hwnd = CreateWindowExA(0,
                                wc.lpszClassName,
                                "HJ_Mouse_Window",
                                0,
                                0,
                                0,
                                0,
                                0,
                                HWND_MESSAGE,
                                NULL,
                                wc.hInstance,
                                NULL);
    SetWindowLongPtr(ctx->hwnd, GWLP_USERDATA, (LONG_PTR) ctx);

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage     = 0x02;
    rid.dwFlags     = RIDEV_INPUTSINK;
    rid.hwndTarget  = ctx->hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

    ctx->running = 1;
    SetEvent(ctx->init_event);

    MSG msg;
    while(ctx->running && GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClassA(wc.lpszClassName, wc.hInstance);
    return 0;
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_enumerate(hj_mouse_info_t *infos,
                                               int              max_count,
                                               int             *out)
{
    if(!infos || max_count <= 0 || !out)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    UINT num_devices = 0;
    if(GetRawInputDeviceList(NULL, &num_devices, sizeof(RAWINPUTDEVICELIST))
           != 0
       || num_devices == 0)
    {
        *out = 0;
        return HJ_MOUSE_SUCCESS;
    }

    PRAWINPUTDEVICELIST raw_list =
        (PRAWINPUTDEVICELIST) malloc(sizeof(RAWINPUTDEVICELIST) * num_devices);
    if(!raw_list)
        return HJ_MOUSE_ERROR_IO;

    GetRawInputDeviceList(raw_list, &num_devices, sizeof(RAWINPUTDEVICELIST));

    int count = 0;
    for(UINT i = 0; i < num_devices && count < max_count; ++i)
    {
        if(raw_list[i].dwType == RIM_TYPEMOUSE)
        {
            memset(&infos[count], 0, sizeof(hj_mouse_info_t));
            snprintf(infos[count].device_path,
                     sizeof(infos[count].device_path),
                     "\\\\?\\RAW_MOUSE_%p",
                     raw_list[i].hDevice);

            hj_safe_strcpy(infos[count].manufacturer,
                           "Windows HID Mouse",
                           sizeof(infos[count].manufacturer));
            hj_safe_strcpy(infos[count].product,
                           "RawInput Mouse Device",
                           sizeof(infos[count].product));

            RID_DEVICE_INFO dev_info;
            dev_info.cbSize = sizeof(RID_DEVICE_INFO);
            UINT info_size  = sizeof(RID_DEVICE_INFO);
            if(GetRawInputDeviceInfoA(raw_list[i].hDevice,
                                      RIDI_DEVICEINFO,
                                      &dev_info,
                                      &info_size)
               > 0)
            {
                infos[count].vendor_id = (uint16_t) dev_info.mouse.dwId;
                infos[count].product_id =
                    (uint16_t) dev_info.mouse.dwNumberOfButtons;
            }
            count++;
        }
    }

    free(raw_list);
    *out = count;
    return HJ_MOUSE_SUCCESS;
}

HJ_MOUSE_API intptr_t hj_mouse_open(const char *device_path)
{
    hj_win32_mouse_ctx_t *ctx =
        (hj_win32_mouse_ctx_t *) calloc(1, sizeof(hj_win32_mouse_ctx_t));
    if(!ctx)
        return (intptr_t) HJ_MOUSE_ERROR_IO;

    InitializeCriticalSection(&ctx->cs);
    ctx->init_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(device_path && strncmp(device_path, "\\\\?\\RAW_MOUSE_", 14) == 0)
    {
        sscanf(device_path + 14, "%p", &ctx->target_raw_device);
    }

    ctx->thread_handle = (HANDLE)
        _beginthreadex(NULL, 0, hj_win32_raw_input_thread, ctx, 0, NULL);
    if(!ctx->thread_handle)
    {
        CloseHandle(ctx->init_event);
        DeleteCriticalSection(&ctx->cs);
        free(ctx);
        return (intptr_t) HJ_MOUSE_ERROR_IO;
    }

    WaitForSingleObject(ctx->init_event, INFINITE);
    CloseHandle(ctx->init_event);

    return (intptr_t) ctx;
}

HJ_MOUSE_API void hj_mouse_close(intptr_t handle)
{
    if(handle <= 0 || handle == (intptr_t) INVALID_HANDLE_VALUE)
        return;

    hj_win32_mouse_ctx_t *ctx = (hj_win32_mouse_ctx_t *) handle;
    ctx->running              = 0;
    if(ctx->hwnd)
    {
        PostMessage(ctx->hwnd, WM_CLOSE, 0, 0);
    }

    if(ctx->thread_handle)
    {
        WaitForSingleObject(ctx->thread_handle, 1000);
        CloseHandle(ctx->thread_handle);
    }

    DeleteCriticalSection(&ctx->cs);
    free(ctx);
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_read_event(intptr_t          handle,
                                                hj_mouse_event_t *event)
{
    if(!event || handle <= 0 || handle == (intptr_t) INVALID_HANDLE_VALUE)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    hj_win32_mouse_ctx_t *ctx = (hj_win32_mouse_ctx_t *) handle;
    hj_mouse_err_t        err = HJ_MOUSE_ERROR_NO_DATA;

    EnterCriticalSection(&ctx->cs);
    if(ctx->tail != ctx->head)
    {
        *event    = ctx->queue[ctx->tail];
        ctx->tail = (ctx->tail + 1) % HJ_WIN32_EVENT_QUEUE_SIZE;
        err       = HJ_MOUSE_SUCCESS;
    }
    LeaveCriticalSection(&ctx->cs);

    return err;
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_set_param(intptr_t handle, int accel)
{
    (void) handle;
    (void) accel;
    return HJ_MOUSE_ERROR_NOT_SUPPORTED;
}

// ---------------- macOS Implement ----------------
#elif defined(__APPLE__)

#include <pthread.h>

#define HJ_MACOS_EVENT_QUEUE_SIZE 128

typedef struct
{
    hj_mouse_event_t queue[HJ_MACOS_EVENT_QUEUE_SIZE];
    int              head;
    int              tail;
    pthread_mutex_t  mutex;
    pthread_t        thread;
    CFRunLoopRef     run_loop;
    IOHIDManagerRef  hid_manager;
    volatile int     running;
    void            *target_device_ptr;
} hj_macos_mouse_ctx_t;

HJ_MOUSE_API void hj_macos_push_event(hj_macos_mouse_ctx_t   *ctx,
                                      const hj_mouse_event_t *evt)
{
    pthread_mutex_lock(&ctx->mutex);
    int next = (ctx->head + 1) % HJ_MACOS_EVENT_QUEUE_SIZE;
    if(next != ctx->tail)
    {
        ctx->queue[ctx->head] = *evt;
        ctx->head             = next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

static void hj_macos_hid_callback(void         *context,
                                  IOReturn      result,
                                  void         *sender,
                                  IOHIDValueRef value)
{
    (void) result;
    (void) sender;
    hj_macos_mouse_ctx_t *ctx = (hj_macos_mouse_ctx_t *) context;
    if(!ctx)
        return;

    IOHIDElementRef element   = IOHIDValueGetElement(value);
    uint32_t        usagePage = IOHIDElementGetUsagePage(element);
    uint32_t        usage     = IOHIDElementGetUsage(element);
    CFIndex         val       = IOHIDValueGetIntegerValue(value);

    hj_mouse_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.timestamp_us = hj_mouse_get_timestamp_us();

    if(usagePage == 0x01)
    {
        if(usage == 0x30)
        {
            evt.type = HJ_MOUSE_EVENT_MOTION;
            evt.dx   = (int32_t) val;
            hj_macos_push_event(ctx, &evt);
        } else if(usage == 0x31)
        {
            evt.type = HJ_MOUSE_EVENT_MOTION;
            evt.dy   = (int32_t) val;
            hj_macos_push_event(ctx, &evt);
        } else if(usage == 0x38)
        {
            evt.type        = HJ_MOUSE_EVENT_WHEEL;
            evt.wheel_delta = (int32_t) val;
            hj_macos_push_event(ctx, &evt);
        }
    }

    else if(usagePage == 0x09)
    {
        evt.type    = HJ_MOUSE_EVENT_BUTTON;
        evt.pressed = (val != 0);
        switch(usage)
        {
            case 1:
                evt.button = HJ_MOUSE_BTN_LEFT;
                break;
            case 2:
                evt.button = HJ_MOUSE_BTN_RIGHT;
                break;
            case 3:
                evt.button = HJ_MOUSE_BTN_MIDDLE;
                break;
            case 4:
                evt.button = HJ_MOUSE_BTN_EXTRA1;
                break;
            case 5:
                evt.button = HJ_MOUSE_BTN_EXTRA2;
                break;
            default:
                evt.button = HJ_MOUSE_BTN_NONE;
                break;
        }
        hj_macos_push_event(ctx, &evt);
    }
}

static void *hj_macos_event_thread(void *arg)
{
    hj_macos_mouse_ctx_t *ctx = (hj_macos_mouse_ctx_t *) arg;
    ctx->run_loop             = CFRunLoopGetCurrent();

    ctx->hid_manager =
        IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(ctx->hid_manager)
    {
        CFMutableDictionaryRef matchingDict =
            CFDictionaryCreateMutable(kCFAllocatorDefault,
                                      0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
        int         page = 0x01, usage = 0x02;
        CFNumberRef pageRef  = CFNumberCreate(NULL, kCFNumberIntType, &page);
        CFNumberRef usageRef = CFNumberCreate(NULL, kCFNumberIntType, &usage);
        CFDictionarySetValue(matchingDict,
                             CFSTR(kIOHIDDeviceUsagePageKey),
                             pageRef);
        CFDictionarySetValue(matchingDict,
                             CFSTR(kIOHIDDeviceUsageKey),
                             usageRef);

        IOHIDManagerSetDeviceMatching(ctx->hid_manager, matchingDict);
        CFRelease(pageRef);
        CFRelease(usageRef);
        CFRelease(matchingDict);

        IOHIDManagerRegisterInputValueCallback(ctx->hid_manager,
                                               hj_macos_hid_callback,
                                               ctx);
        IOHIDManagerScheduleWithRunLoop(ctx->hid_manager,
                                        ctx->run_loop,
                                        kCFRunLoopDefaultMode);
        IOHIDManagerOpen(ctx->hid_manager, kIOHIDOptionsTypeNone);
    }

    ctx->running = 1;
    CFRunLoopRun();

    if(ctx->hid_manager)
    {
        IOHIDManagerUnscheduleFromRunLoop(ctx->hid_manager,
                                          ctx->run_loop,
                                          kCFRunLoopDefaultMode);
        IOHIDManagerClose(ctx->hid_manager, kIOHIDOptionsTypeNone);
        CFRelease(ctx->hid_manager);
    }
    return NULL;
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_enumerate(hj_mouse_info_t *infos,
                                               int              max_count,
                                               int             *out)
{
    if(!infos || max_count <= 0 || !out)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    IOHIDManagerRef manager =
        IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!manager)
        return HJ_MOUSE_ERROR_IO;

    CFMutableDictionaryRef matchingDict =
        CFDictionaryCreateMutable(kCFAllocatorDefault,
                                  0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);

    int         usagePage = 0x01, usage = 0x02;
    CFNumberRef pageRef =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
    CFNumberRef usageRef =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    CFDictionarySetValue(matchingDict,
                         CFSTR(kIOHIDDeviceUsagePageKey),
                         pageRef);
    CFDictionarySetValue(matchingDict, CFSTR(kIOHIDDeviceUsageKey), usageRef);

    IOHIDManagerSetDeviceMatching(manager, matchingDict);
    CFRelease(pageRef);
    CFRelease(usageRef);

    CFSetRef deviceSet = IOHIDManagerCopyDevices(manager);
    if(!deviceSet)
    {
        CFRelease(matchingDict);
        CFRelease(manager);
        *out = 0;
        return HJ_MOUSE_SUCCESS;
    }

    CFIndex         num_devices = CFSetGetCount(deviceSet);
    IOHIDDeviceRef *devices =
        (IOHIDDeviceRef *) malloc(sizeof(IOHIDDeviceRef) * num_devices);
    CFSetGetValues(deviceSet, (const void **) devices);

    int count = 0;
    for(CFIndex i = 0; i < num_devices && count < max_count; ++i)
    {
        memset(&infos[count], 0, sizeof(hj_mouse_info_t));
        snprintf(infos[count].device_path,
                 sizeof(infos[count].device_path),
                 "IOHIDDevice_%p",
                 (void *) devices[i]);

        CFStringRef productRef =
            (CFStringRef) IOHIDDeviceGetProperty(devices[i],
                                                 CFSTR(kIOHIDProductKey));
        if(productRef)
        {
            CFStringGetCString(productRef,
                               infos[count].product,
                               sizeof(infos[count].product),
                               kCFStringEncodingUTF8);
        } else
        {
            hj_safe_strcpy(infos[count].product,
                           "macOS HID Mouse",
                           sizeof(infos[count].product));
        }

        CFNumberRef vendorIdRef =
            (CFNumberRef) IOHIDDeviceGetProperty(devices[i],
                                                 CFSTR(kIOHIDVendorIDKey));
        if(vendorIdRef)
        {
            int vid = 0;
            CFNumberGetValue(vendorIdRef, kCFNumberIntType, &vid);
            infos[count].vendor_id = (uint16_t) vid;
            snprintf(infos[count].manufacturer,
                     sizeof(infos[count].manufacturer),
                     "VID_%04x",
                     vid);
        }

        count++;
    }

    free(devices);
    CFRelease(deviceSet);
    CFRelease(matchingDict);
    CFRelease(manager);
    *out = count;
    return HJ_MOUSE_SUCCESS;
}

HJ_MOUSE_API intptr_t hj_mouse_open(const char *device_path)
{
    hj_macos_mouse_ctx_t *ctx =
        (hj_macos_mouse_ctx_t *) calloc(1, sizeof(hj_macos_mouse_ctx_t));
    if(!ctx)
        return (intptr_t) HJ_MOUSE_ERROR_IO;

    pthread_mutex_init(&ctx->mutex, NULL);

    if(device_path && strncmp(device_path, "IOHIDDevice_", 12) == 0)
    {
        sscanf(device_path + 12, "%p", &ctx->target_device_ptr);
    }

    if(pthread_create(&ctx->thread, NULL, hj_macos_event_thread, ctx) != 0)
    {
        pthread_mutex_destroy(&ctx->mutex);
        free(ctx);
        return (intptr_t) HJ_MOUSE_ERROR_IO;
    }

    while(!ctx->running)
    {
        usleep(1000);
    }

    return (intptr_t) ctx;
}

HJ_MOUSE_API void hj_mouse_close(intptr_t handle)
{
    if(handle <= 0)
        return;

    hj_macos_mouse_ctx_t *ctx = (hj_macos_mouse_ctx_t *) handle;
    ctx->running              = 0;

    if(ctx->run_loop)
    {
        CFRunLoopStop(ctx->run_loop);
    }

    pthread_join(ctx->thread, NULL);
    pthread_mutex_destroy(&ctx->mutex);
    free(ctx);
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_read_event(intptr_t          handle,
                                                hj_mouse_event_t *event)
{
    if(!event || handle <= 0)
        return HJ_MOUSE_ERROR_INVALID_PARAM;

    hj_macos_mouse_ctx_t *ctx = (hj_macos_mouse_ctx_t *) handle;
    hj_mouse_err_t        err = HJ_MOUSE_ERROR_NO_DATA;

    pthread_mutex_lock(&ctx->mutex);
    if(ctx->tail != ctx->head)
    {
        *event    = ctx->queue[ctx->tail];
        ctx->tail = (ctx->tail + 1) % HJ_MACOS_EVENT_QUEUE_SIZE;
        err       = HJ_MOUSE_SUCCESS;
    }
    pthread_mutex_unlock(&ctx->mutex);

    return err;
}

HJ_MOUSE_API hj_mouse_err_t hj_mouse_set_param(intptr_t handle, int accel)
{
    (void) handle;
    (void) accel;
    return HJ_MOUSE_ERROR_NOT_SUPPORTED;
}

#endif

#ifdef __cplusplus
}
#endif

#endif // HJ_MOUSE_IMPL && !HJ_MOUSE_IMPL_DONE