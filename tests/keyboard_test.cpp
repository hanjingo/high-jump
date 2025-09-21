#include <gtest/gtest.h>
#include <libcpp/hardware/keyboard.h>

TEST(KeyboardTest, EnumerateKeyboards) 
{
    keyboard_info_t infos[8] = {0};
    int n = keyboard_enumerate(infos, 8);
    EXPECT_GE(n, 0);
    for (int i = 0; i < n; ++i) 
    {
        EXPECT_GT(strlen(infos[i].manufacturer), 0);
        EXPECT_GT(strlen(infos[i].product), 0);
    }
}

TEST(KeyboardTest, OpenCloseKeyboard) 
{
    keyboard_info_t infos[4] = {0};
    int n = keyboard_enumerate(infos, 4);
    if (n > 0 && strlen(infos[0].device_path) > 0) 
    {
        int handle = keyboard_open(infos[0].device_path);
        EXPECT_GE(handle, 0);
        keyboard_close(handle);
    } 
    else 
    {
        SUCCEED() << "No keyboard device to open";
    }
}

TEST(KeyboardTest, ReadKeyEvent) 
{
    keyboard_info_t infos[4] = {0};
    int n = keyboard_enumerate(infos, 4);
    int handle = -1;
    if (n > 0 && strlen(infos[0].device_path) > 0)
        handle = keyboard_open(infos[0].device_path);
    else
        handle = keyboard_open(NULL);

    key_event_t event = {0};
    int ret = keyboard_read_event(handle, &event);
    EXPECT_TRUE(ret == 0 || ret == -1);
    keyboard_close(handle);
}

TEST(KeyboardTest, SetRepeat) 
{
    int handle = keyboard_open(NULL);
    int ret = keyboard_set_repeat(handle, 500, 30);
#if defined(__APPLE__)
    EXPECT_EQ(ret, -1);

#else
    EXPECT_TRUE(ret == 0 || ret == -1);

#endif

    keyboard_close(handle);
}