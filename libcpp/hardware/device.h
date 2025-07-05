#ifndef DEVICE_H
#define DEVICE_H

#include <hidapi/hidapi.h>
#include <cstring>

typedef hid_device_info device_info_t;
typedef bool(*device_range_fn)(device_info_t*);

void device_range(const device_range_fn fn)
{
    device_info_t* head = hid_enumerate(0x00, 0x00);
    device_info_t* info = head;
    while (info)
    {
        if (!fn(info))
        {
            break;
        }
        info = info->next;
    }
    hid_free_enumeration(head);
}

#endif