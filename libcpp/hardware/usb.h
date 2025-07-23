#ifndef USB_H
#define USB_H

#include <hidapi/hidapi.h>
#include <cstring>

typedef hid_device_info usb_info_t;
typedef bool(*usb_device_range_fn)(usb_info_t*);

// ----------------------------- USB API define ------------------------------------
void usb_device_range(const usb_device_range_fn fn);

// ----------------------------- USB API implement ------------------------------------
void usb_device_range(const usb_device_range_fn fn)
{
    usb_info_t* head = hid_enumerate(0x00, 0x00);
    usb_info_t* info = head;
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