#include <codecvt>
#include <locale>

#include <gtest/gtest.h>
#include <libcpp/hardware/usb.h>

std::string ws2s(const wchar_t* wstr)
{
    if (!wstr)
        return "";
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

bool dev_range(usb_info_t* info)
{
    if (!info)
        return false;

    std::cout << "{"
              << "path=" << info->path << ", vendor_id=" << info->vendor_id
              << ", product_id=" << info->product_id
              << ", serial_number=" << ws2s(info->serial_number)
              << ", release_number=" << info->release_number
              << ", manufacturer_string=" << ws2s(info->manufacturer_string)
              << ", product_string=" << ws2s(info->product_string)
              << ", usage_page=" << info->usage_page
              << ", usage=" << info->usage
              << ", interface_number=" << info->interface_number
              << ", bus_type=" << (int)(info->bus_type) << "}" << std::endl;
    return true;
}

TEST(usb, usb_device_range)
{
    usb_device_range(dev_range, default_usb_device_filter);
}

TEST(usb, usb_device_count)
{
    int count = usb_device_count(default_usb_device_filter);
    std::cout << "USB device count: " << count << std::endl;
}