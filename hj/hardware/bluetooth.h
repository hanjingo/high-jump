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

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <hidapi/hidapi.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_BLUETOOTH_API
#if defined(HJ_BLUETOOTH_STATIC)
#define HJ_BLUETOOTH_API static inline
#else
#define HJ_BLUETOOTH_API extern
#endif
#endif

typedef enum
{
    HJ_BLUETOOTH_OK                 = 0,
    HJ_BLUETOOTH_FAILED             = -1,
    HJ_BLUETOOTH_ERROR_NULL_POINTER = -2,
    HJ_BLUETOOTH_ERROR_ENUM_FAILED  = -3
} hj_bluetooth_err_t;

typedef struct hid_device_info hj_bluetooth_info_t;

typedef bool (*hj_bluetooth_device_range_fn)(hj_bluetooth_info_t *device,
                                             void                *user_data);

typedef bool (*hj_bluetooth_device_filter_fn)(
    const hj_bluetooth_info_t *device);

HJ_BLUETOOTH_API bool
hj_default_bluetooth_device_filter(const hj_bluetooth_info_t *device);

HJ_BLUETOOTH_API hj_bluetooth_err_t
hj_bluetooth_device_range(hj_bluetooth_device_range_fn  fn,
                          hj_bluetooth_device_filter_fn filter,
                          void                         *user_data);

HJ_BLUETOOTH_API int
hj_bluetooth_device_count(hj_bluetooth_device_filter_fn filter);

#ifdef __cplusplus
}
#endif

#endif // BLUETOOTH_H


// --------------------- Implementation -------------------------
// This section uses STB-style single-header implementation.
// To include implementation, define HJ_BLUETOOTH_IMPL before including
// this header in ONE C/C++ source file.
#if (defined(HJ_BLUETOOTH_IMPL) || defined(HJ_BLUETOOTH_STATIC))               \
    && !defined(HJ_BLUETOOTH_IMPL_DONE)
#define HJ_BLUETOOTH_IMPL_DONE

#ifdef __cplusplus
extern "C" {
#endif

HJ_BLUETOOTH_API bool
hj_default_bluetooth_device_filter(const hj_bluetooth_info_t *device)
{
    if(!device)
        return false;

    if(device->bus_type == HID_API_BUS_BLUETOOTH)
        return true;

    static const unsigned short vendor_id_list[] = {
        0x0A5C, // Broadcom
        0x8087, // Intel
        0x0CF3, // Qualcomm Atheros
        0x0489, // Foxconn / Hon Hai
        0x05AC, // Apple
        0x046D, // Logitech
        0x045E, // Microsoft
        0x1286, // Marvell
        0x0A12, // Cambridge Silicon Radio (CSR)
        0x04CA, // Lite-On
        0x0930, // Toshiba
        0x13D3, // IMC Networks
        0x0BDA  // Realtek
    };

    size_t vendor_count = sizeof(vendor_id_list) / sizeof(vendor_id_list[0]);
    for(size_t i = 0; i < vendor_count; ++i)
    {
        if(device->vendor_id == vendor_id_list[i])
        {
            if(device->bus_type != HID_API_BUS_USB
               && device->bus_type != HID_API_BUS_SPI)
            {
                return true;
            }
        }
    }

    return false;
}

HJ_BLUETOOTH_API hj_bluetooth_err_t
hj_bluetooth_device_range(hj_bluetooth_device_range_fn  fn,
                          hj_bluetooth_device_filter_fn filter,
                          void                         *user_data)
{
    if(!fn)
        return HJ_BLUETOOTH_ERROR_NULL_POINTER;

    if(hid_init() != 0)
        return HJ_BLUETOOTH_ERROR_ENUM_FAILED;

    hj_bluetooth_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
    {
        const wchar_t     *err_str = hid_error(NULL);
        hj_bluetooth_err_t ret     = (err_str && *err_str != L'\0')
                                         ? HJ_BLUETOOTH_ERROR_ENUM_FAILED
                                         : HJ_BLUETOOTH_OK;
        hid_exit();
        return ret;
    }

    hj_bluetooth_device_filter_fn target_filter =
        filter ? filter : hj_default_bluetooth_device_filter;

    for(hj_bluetooth_info_t *info = head; info != NULL; info = info->next)
    {
        if(!target_filter(info))
            continue;

        if(!fn(info, user_data))
            break;
    }

    hid_free_enumeration(head);
    hid_exit();
    return HJ_BLUETOOTH_OK;
}

HJ_BLUETOOTH_API int
hj_bluetooth_device_count(hj_bluetooth_device_filter_fn filter)
{
    if(hid_init() != 0)
        return -1;

    hj_bluetooth_info_t *head = hid_enumerate(0x00, 0x00);
    if(!head)
    {
        const wchar_t *err_str = hid_error(NULL);
        int            ret     = (err_str && *err_str != L'\0') ? -1 : 0;
        hid_exit();
        return ret;
    }

    hj_bluetooth_device_filter_fn target_filter =
        filter ? filter : hj_default_bluetooth_device_filter;

    int count = 0;
    for(hj_bluetooth_info_t *info = head; info != NULL; info = info->next)
    {
        if(target_filter(info))
        {
            count++;
        }
    }

    hid_free_enumeration(head);
    hid_exit();
    return count;
}

#ifdef __cplusplus
}
#endif

#endif // HJ_BLUETOOTH_IMPL && !HJ_BLUETOOTH_IMPL_DONE