#include <gtest/gtest.h>
#include <libcpp/hardware/device.h>

bool dev_range(device_info_t* info)
{
    std::cout << "{"
              << "path=" << std::string(info->path)
              << ", vendor_id=" << info->vendor_id
              << ", product_id=" << info->product_id
              << ", serial_number=" << info->serial_number
              << ", release_number=" << info->release_number
              << ", manufacturer_string=" << info->manufacturer_string
              << ", product_string=" << info->product_string
              << ", usage_page=" << info->usage_page
              << ", usage=" << info->usage
              << ", interface_number=" << info->interface_number
              << ", bus_type=" << (int)(info->bus_type)
              << "}"
              << std::endl;
    return true;
}

TEST(device, range)
{
    device_range(dev_range);
}