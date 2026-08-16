#define HJ_BLUETOOTH_IMPL
#include <gtest/gtest.h>
#include <hj/hardware/bluetooth.h>

static bool print_bluetooth_device_info(hj_bluetooth_info_t *device,
                                        void                *user_data)
{
    (void) user_data;

    if(!device)
        return false;

    printf("=== Bluetooth Device ===\n");
    printf("Path: %s\n", device->path ? device->path : "Unknown");
    printf("Vendor ID: 0x%04X\n", device->vendor_id);
    printf("Product ID: 0x%04X\n", device->product_id);
    printf("Serial: %ls\n",
           device->serial_number ? device->serial_number : L"Unknown");
    printf("Release: 0x%04X\n", device->release_number);
    printf("Manufacturer: %ls\n",
           device->manufacturer_string ? device->manufacturer_string
                                       : L"Unknown");
    printf("Product: %ls\n",
           device->product_string ? device->product_string : L"Unknown");
    printf("Usage Page: 0x%04X\n", device->usage_page);
    printf("Usage: 0x%04X\n", device->usage);
    printf("Interface: %d\n", device->interface_number);
    printf("\n");

    return true;
}

TEST(BluetoothFilter, NullDevicePointer)
{
    EXPECT_FALSE(hj_default_bluetooth_device_filter(nullptr));
}

TEST(BluetoothFilter, BluetoothBusTypeMatch)
{
    hj_bluetooth_info_t dev = {};
    dev.bus_type            = HID_API_BUS_BLUETOOTH;
    dev.vendor_id           = 0x0000;

    EXPECT_TRUE(hj_default_bluetooth_device_filter(&dev));
}

TEST(BluetoothFilter, VendorIdMatchNonUsbSpi)
{
    hj_bluetooth_info_t dev = {};
    dev.bus_type            = HID_API_BUS_UNKNOWN;
    dev.vendor_id           = 0x05AC;

    EXPECT_TRUE(hj_default_bluetooth_device_filter(&dev));
}

TEST(BluetoothFilter, VendorIdMatchButUsbOrSpiBus)
{
    hj_bluetooth_info_t dev_usb = {};
    dev_usb.bus_type            = HID_API_BUS_USB;
    dev_usb.vendor_id           = 0x05AC;
    EXPECT_FALSE(hj_default_bluetooth_device_filter(&dev_usb));

    hj_bluetooth_info_t dev_spi = {};
    dev_spi.bus_type            = HID_API_BUS_SPI;
    dev_spi.vendor_id           = 0x8087;
    EXPECT_FALSE(hj_default_bluetooth_device_filter(&dev_spi));
}

TEST(BluetoothFilter, NonMatchingVendorAndBus)
{
    hj_bluetooth_info_t dev = {};
    dev.bus_type            = HID_API_BUS_USB;
    dev.vendor_id           = 0x1234;

    EXPECT_FALSE(hj_default_bluetooth_device_filter(&dev));
}

TEST(BluetoothRange, NullCallbackReturnsError)
{
    hj_bluetooth_err_t err =
        hj_bluetooth_device_range(nullptr,
                                  hj_default_bluetooth_device_filter,
                                  nullptr);
    EXPECT_EQ(err, HJ_BLUETOOTH_ERROR_NULL_POINTER);
}

TEST(BluetoothRange, NullFilterFallbackToDefault)
{
    hj_bluetooth_err_t err =
        hj_bluetooth_device_range(print_bluetooth_device_info,
                                  nullptr,
                                  nullptr);
    EXPECT_EQ(err, HJ_BLUETOOTH_OK);
}

TEST(BluetoothRange, EarlyTerminationOnCallbackFalse)
{
    int visited_count = 0;

    hj_bluetooth_err_t err = hj_bluetooth_device_range(
        [](hj_bluetooth_info_t *, void *user_data) -> bool {
            auto *count = static_cast<int *>(user_data);
            (*count)++;
            return false;
        },
        [](const hj_bluetooth_info_t *) -> bool { return true; },
        &visited_count);

    EXPECT_EQ(err, HJ_BLUETOOTH_OK);
    EXPECT_LE(visited_count, 1);
}

TEST(BluetoothRange, CustomFilterRejectAll)
{
    int visited_count = 0;

    hj_bluetooth_err_t err = hj_bluetooth_device_range(
        [](hj_bluetooth_info_t *, void *user_data) -> bool {
            auto *count = static_cast<int *>(user_data);
            (*count)++;
            return true;
        },
        [](const hj_bluetooth_info_t *) -> bool { return false; },
        &visited_count);

    EXPECT_EQ(err, HJ_BLUETOOTH_OK);
    EXPECT_EQ(visited_count, 0);
}

TEST(BluetoothCount, NullFilterFallback)
{
    int count = hj_bluetooth_device_count(nullptr);
    EXPECT_GE(count, 0);
}

TEST(BluetoothCount, CountMatchesRangeIteration)
{
    int range_counted = 0;

    hj_bluetooth_err_t err = hj_bluetooth_device_range(
        [](hj_bluetooth_info_t *, void *user_data) -> bool {
            auto *count = static_cast<int *>(user_data);
            (*count)++;
            return true;
        },
        hj_default_bluetooth_device_filter,
        &range_counted);

    EXPECT_EQ(err, HJ_BLUETOOTH_OK);

    int count = hj_bluetooth_device_count(hj_default_bluetooth_device_filter);
    EXPECT_EQ(range_counted, count);
}