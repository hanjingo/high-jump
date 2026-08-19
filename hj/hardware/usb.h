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

typedef hid_device_info usb_info_t;
typedef bool (*usb_device_range_fn)(usb_info_t *device);
typedef bool (*usb_device_filter_fn)(const usb_info_t *device);

HJ_USB_API bool default_usb_device_filter(const usb_info_t *device)
{
    if(!device)
        return false;

    if(device->bus_type == HID_API_BUS_USB)
        return true;

    return false;
}

// ------------------------ ROM API Declarations ------------------------
HJ_USB_API void usb_device_range(usb_device_range_fn  fn,
                                 usb_device_filter_fn filter);
HJ_USB_API int  usb_device_count(usb_device_filter_fn filter);

#ifdef __cplusplus
}
#endif

#endif // USB_H

// --------------------- Implementation -------------------------
// To include implementation, define HJ_USB_IMPL before including
// this header in ONE C/C++ source file.
#if (defined(HJ_USB_IMPL) || defined(HJ_USB_STATIC))                           \
    && !defined(HJ_USB_IMPL_DONE)
#define HJ_USB_IMPL_DONE

#ifdef __cplusplus
extern "C" {
#endif

HJ_USB_API void usb_device_range(usb_device_range_fn  fn,
                                 usb_device_filter_fn filter)
{
    if(!fn)
        return;

    usb_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
        return;

    usb_info_t *info;
    for(info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;

        if(!fn(info))
            break;
    }

    hid_free_enumeration(head);
}

HJ_USB_API int usb_device_count(usb_device_filter_fn filter)
{
    usb_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
        return 0;

    int         count = 0;
    usb_info_t *info;
    for(info = head; info; info = info->next)
    {
        if(filter && !filter(info))
            continue;

        count++;
    }

    hid_free_enumeration(head);
    return count;
}

#ifdef __cplusplus
}
#endif

#endif /* USB_H */