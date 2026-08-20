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

#ifndef USB_H
#define USB_H

#include <hidapi/hidapi.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_USB_API
#if defined(HJ_USB_STATIC)
#define HJ_USB_API static inline
#else
#define HJ_USB_API extern
#endif
#endif

typedef hid_device_info hj_usb_info_t;
typedef bool (*hj_usb_device_range_fn)(const hj_usb_info_t *device);
typedef bool (*hj_usb_device_filter_fn)(const hj_usb_info_t *device);

// ------------------------ API Declarations ------------------------
HJ_USB_API int  hj_usb_init(void);
HJ_USB_API int  hj_usb_shutdown(void);
HJ_USB_API void hj_usb_device_range(hj_usb_device_range_fn  fn,
                                    hj_usb_device_filter_fn filter);
HJ_USB_API int  hj_usb_device_count(hj_usb_device_filter_fn filter);
HJ_USB_API bool hj_default_usb_device_filter(const hj_usb_info_t *device);

#ifdef __cplusplus
}
#endif

#endif // USB_H

// --------------------- Implementation -------------------------
#if (defined(HJ_USB_IMPL) || defined(HJ_USB_STATIC))                           \
    && !defined(HJ_USB_IMPL_DONE)
#define HJ_USB_IMPL_DONE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

static int  g_hj_user_init_count    = 0;
static int  g_hj_internal_init_flag = 0;
static void usb_atexit_handler(void)
{
    if(g_hj_internal_init_flag)
    {
        hid_exit();
        g_hj_internal_init_flag = 0;
    }
}

static void internal_hid_ensure_init(void)
{
    if(!g_hj_internal_init_flag)
    {
        if(hid_init() == 0)
        {
            g_hj_internal_init_flag = 1;
            atexit(usb_atexit_handler);
        }
    }
}

HJ_USB_API int hj_usb_init(void)
{
    if(g_hj_user_init_count++ == 0)
    {
        if(hid_init() != 0)
        {
            g_hj_user_init_count = 0;
            return -1;
        }
    }
    return 0;
}

HJ_USB_API int hj_usb_shutdown(void)
{
    if(g_hj_user_init_count > 0 && --g_hj_user_init_count == 0)
    {
        return hid_exit();
    }
    return 0;
}

HJ_USB_API void hj_usb_device_range(hj_usb_device_range_fn  fn,
                                    hj_usb_device_filter_fn filter)
{
    if(!fn)
        return;

    internal_hid_ensure_init();

    hj_usb_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
        return;

    const hj_usb_info_t *info;
    for(info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;

        if(!fn(info))
            break;
    }

    hid_free_enumeration(head);
}

HJ_USB_API int hj_usb_device_count(hj_usb_device_filter_fn filter)
{
    internal_hid_ensure_init();

    hj_usb_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
        return 0;

    int                  count = 0;
    const hj_usb_info_t *info;
    for(info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;

        count++;
    }

    hid_free_enumeration(head);
    return count;
}

HJ_USB_API bool hj_default_usb_device_filter(const hj_usb_info_t *device)
{
    if(!device)
        return false;

    if(device->bus_type == HID_API_BUS_USB)
        return true;

    return false;
}

#ifdef __cplusplus
}
#endif

#endif /* USB_H */