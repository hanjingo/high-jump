#include <gtest/gtest.h>
#include <hj/hardware/mouse.h>

TEST(MouseTest, EnumerateMouse) 
{
    mouse_info_t infos[8] = {0};
    int n = mouse_enumerate(infos, 8);
    EXPECT_GE(n, 0);
    if (n == 0) {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    for (int i = 0; i < n; ++i) 
    {
        EXPECT_GT(strlen(infos[i].manufacturer), 0);
        EXPECT_GT(strlen(infos[i].product), 0);
    }
}

TEST(MouseTest, OpenCloseMouse) 
{
    mouse_info_t infos[4] = {0};
    int n = mouse_enumerate(infos, 4);
    if (n == 0) {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    int handle = -1;
    if (strlen(infos[0].device_path) > 0)
        handle = mouse_open(infos[0].device_path);
    else
        handle = mouse_open(NULL);

    EXPECT_GE(handle, 0);
    mouse_close(handle);
}

TEST(MouseTest, ReadMouseEvent) 
{
    mouse_info_t infos[2] = {0};
    int n = mouse_enumerate(infos, 2);
    if (n == 0) {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    int handle = -1;
    if (strlen(infos[0].device_path) > 0)
        handle = mouse_open(infos[0].device_path);
    else
        handle = mouse_open(NULL);

    mouse_event_t event = {0};
    int ret = mouse_read_event(handle, &event);
    EXPECT_TRUE(ret == 0 || ret == -1);
    mouse_close(handle);
}

TEST(MouseTest, SetMouseParam) 
{
    mouse_info_t infos[1] = {0};
    int n = mouse_enumerate(infos, 1);
    if (n == 0) {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    int handle = mouse_open(NULL);
    int ret = mouse_set_param(handle, 10);
    EXPECT_TRUE(ret == 0 || ret == -1);
    mouse_close(handle);
}