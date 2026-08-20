#include <gtest/gtest.h>

#define HJ_USB_IMPL
#include <hj/hardware/usb.h>

bool dev_range_callback(const hj_usb_info_t *info)
{
    EXPECT_NE(info, nullptr);
    return true;
}

TEST(usb, usb_device_range)
{
    hj_usb_device_range(dev_range_callback, hj_default_usb_device_filter);
}

TEST(usb, usb_device_count)
{
    int count = hj_usb_device_count(hj_default_usb_device_filter);
    EXPECT_GE(count, 0);

    // EXPECT_GT(count, 0);
}