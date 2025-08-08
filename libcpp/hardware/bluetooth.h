/*
 *  This file is part of libcpp.
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
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef hid_device_info bluetooth_info_t;
typedef bool(*bluetooth_device_range_fn)(bluetooth_info_t* device);
typedef bool(*bluetooth_device_filter_fn)(const bluetooth_info_t* device);

static bool default_bluetooth_device_filter(const bluetooth_info_t* device)
{
    static unsigned short vendor_id_list[] = {
        0x0A5C, // Broadcom
        0x8087, // Intel
        0x0CF3, // Qualcomm Atheros
        0x0489, // Foxconn / Hon Hai
        0x05AC, // Apple
        0x046D, // Logitech
        0x045E, // Microsoft
        0x1286, // Marvell
        0x0A12, // Cambridge Silicon Radio (CSR)
        0x04CA, // Lite-On Technology
        0x0930, // Toshiba
        0x13D3, // IMC Networks
        0x0BDA, // Realtek

        // for more vendor IDs, you can add them here
    };
    if (!device) 
        return false;

    for (int i = 0; i < sizeof(vendor_id_list) / sizeof(vendor_id_list[0]); ++i) 
    {
        if (device->vendor_id == vendor_id_list[i])
            return true;
    }

    if (device->bus_type == HID_API_BUS_BLUETOOTH) 
        return true;

    return false;
}

// --------------------------------- Bluetooth API ----------------------------------------
static void bluetooth_device_range(bluetooth_device_range_fn fn, bluetooth_device_filter_fn filter)
{
    if (!fn)
        return;

    bluetooth_info_t* head = hid_enumerate(0x00, 0x00);
    if (!head)
        return;
    
    for (bluetooth_info_t* info = head; info; info = info->next)
    {
        if (!filter || !filter(info))
            continue;

        if (!fn(info))
            break;
    }
    
    hid_free_enumeration(head);
}

static int bluetooth_device_count(bluetooth_device_filter_fn filter)
{
    bluetooth_info_t* head = hid_enumerate(0x00, 0x00);
    if (!head)
        return 0;
    
    int count = 0;
    for (bluetooth_info_t* info = head; info; info = info->next)
    {
        if (filter && !filter(info))
            continue;

        count++;
    }
    
    hid_free_enumeration(head);
    return count;
}

#ifdef __cplusplus
}
#endif

#endif // BLUETOOTH_H