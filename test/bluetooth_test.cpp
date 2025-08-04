#include <gtest/gtest.h>
#include <libcpp/hardware/bluetooth.h>

bool print_bluetooth_device_info(bluetooth_info_t* device)
{
    if (!device)
        return false;

    printf("=== Bluetooth Device ===\n");
    printf("Path: %s\n", device->path ? device->path : "Unknown");
    printf("Vendor ID: 0x%04X\n", device->vendor_id);
    printf("Product ID: 0x%04X\n", device->product_id);
    printf("Serial: %ls\n",
           device->serial_number ? device->serial_number : L"Unknown");
    printf("Release: 0x%04X\n", device->release_number);
    printf(
        "Manufacturer: %ls\n",
        device->manufacturer_string ? device->manufacturer_string : L"Unknown");
    printf("Product: %ls\n",
           device->product_string ? device->product_string : L"Unknown");
    printf("Usage Page: 0x%04X\n", device->usage_page);
    printf("Usage: 0x%04X\n", device->usage);
    printf("Interface: %d\n", device->interface_number);
    printf("\n");

    return true;
}

TEST(bluetooth, bluetooth_device_range)
{
    bluetooth_device_range(print_bluetooth_device_info,
                           default_bluetooth_device_filter);
}

TEST(bluetooth, bluetooth_device_count)
{
    int count = bluetooth_device_count(default_bluetooth_device_filter);
    printf("Bluetooth Device Count: %d\n", count);
}