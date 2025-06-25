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

void device_find_if_path_contains(device_info_t** devices, unsigned long& len, const char* str)
{
    unsigned long nfind = 0;
    device_info_t* info = hid_enumerate(0x00, 0x00);
    while (info && nfind < len)
    {
        if (strstr(info->path, str))
        {
            devices[nfind] = info;
            nfind++;
        }

        info = info->next;
    }

    len = nfind;
}

void device_find_if_vendor_id_equal(device_info_t** devices, unsigned long& len, unsigned short id)
{
    unsigned long nfind = 0;
    device_info_t* info = hid_enumerate(0x00, 0x00);
    while (info && nfind < len)
    {
        if (info->vendor_id == id)
        {
            devices[nfind] = info;
            nfind++;
        }

        info = info->next;
    }

    len = nfind;
}

void device_find_if_product_id_equal(device_info_t** devices, unsigned long& len, unsigned short id)
{
    unsigned long nfind = 0;
    device_info_t* info = hid_enumerate(0x00, 0x00);
    while (info && nfind < len)
    {
        if (info->product_id == id)
        {
            devices[nfind] = info;
            nfind++;
        }

        info = info->next;
    }

    len = nfind;
}

#endif