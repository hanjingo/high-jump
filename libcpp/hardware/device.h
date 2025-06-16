#ifndef DEVICE_H
#define DEVICE_H

#include <hidapi/hidapi.h>
#include <cstring>

typedef hid_device_info device_info_t;
typedef bool(*device_range_fn)(device_info_t*);

void device_range(const device_range_fn fn)
{
    device_info_t* info = hid_enumerate(0x00, 0x00);
    while (info)
    {
        if (!fn(info))
            return;

        info = info->next;
    }
}

void device_find_if_path_contains(const char* path, device_info_t** devices, unsigned long& len)
{
    unsigned long nfind = 0;
    device_info_t* info = hid_enumerate(0x00, 0x00);
    while (info && nfind < len)
    {
        if (strstr(info->path, path))
        {
            devices[nfind] = info;
            nfind++;
        }

        info = info->next;
    }

    len = nfind;
}

#endif