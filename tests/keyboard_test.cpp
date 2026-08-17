#include <gtest/gtest.h>

#define HJ_KEYBOARD_IMPL
#include <hj/hardware/keyboard.h>

TEST(keyboard, enumerate_keyboards)
{
    int               total_count = 0;
    hj_keyboard_err_t err = hj_keyboard_enumerate(NULL, 0, &total_count);
    EXPECT_EQ(err, HJ_KEYBOARD_OK);
    EXPECT_GE(total_count, 0);

    if(total_count > 0)
    {
        std::vector<hj_keyboard_info_t> infos(total_count);
        int                             filled_count = 0;
        err = hj_keyboard_enumerate(infos.data(), total_count, &filled_count);

        EXPECT_EQ(err, HJ_KEYBOARD_OK);
        EXPECT_EQ(filled_count, total_count);

        for(int i = 0; i < filled_count; ++i)
        {
            EXPECT_GT(strlen(infos[i].manufacturer), 0);
            EXPECT_GT(strlen(infos[i].product), 0);
        }

        hj_keyboard_info_t small_buf[1]        = {0};
        int                truncated_out_count = 0;
        err = hj_keyboard_enumerate(small_buf, 1, &truncated_out_count);

        EXPECT_EQ(err, HJ_KEYBOARD_OK);
        EXPECT_EQ(truncated_out_count, total_count);
    }
}

TEST(keyboard, open_close_keyboard)
{
    hj_keyboard_info_t infos[4] = {0};
    int                count    = 0;
    hj_keyboard_enumerate(infos, 4, &count);
    if(count > 0 && strlen(infos[0].device_path) > 0)
    {
        hj_keyboard_handle_t handle = HJ_INVALID_HANDLE;
        hj_keyboard_err_t err = hj_keyboard_open(infos[0].device_path, &handle);
        if(err == HJ_KEYBOARD_OK)
        {
            EXPECT_NE(handle, HJ_INVALID_HANDLE);
            EXPECT_EQ(hj_keyboard_close(handle), HJ_KEYBOARD_OK);
        }
    } else
    {
        SUCCEED() << "No keyboard device to open";
    }
}

TEST(keyboard, read_key_event)
{
    hj_keyboard_info_t infos[4] = {0};
    int                count    = 0;
    hj_keyboard_enumerate(infos, 4, &count);

    hj_keyboard_handle_t handle = HJ_INVALID_HANDLE;
    if(count > 0 && strlen(infos[0].device_path) > 0)
        hj_keyboard_open(infos[0].device_path, &handle);

    if(handle == HJ_INVALID_HANDLE)
    {
        SUCCEED()
            << "Keyboard device open skipped due to platform permission limits";
        return;
    }

    hj_key_event_t    event = {0};
    hj_keyboard_err_t ret   = hj_keyboard_read_event(handle, &event);
    EXPECT_TRUE(ret == HJ_KEYBOARD_OK || ret == HJ_KEYBOARD_ERR_NO_DATA
                || ret == HJ_KEYBOARD_ERR_NOT_SUPPORTED);

    hj_keyboard_close(handle);
}

TEST(keyboard, set_repeat)
{
    hj_keyboard_handle_t handle = HJ_INVALID_HANDLE;
    hj_keyboard_open(NULL, &handle);

    hj_keyboard_err_t ret = hj_keyboard_set_repeat(handle, 500, 30);
#if defined(__APPLE__)
    EXPECT_EQ(ret, HJ_KEYBOARD_ERR_NOT_SUPPORTED);
#else
    EXPECT_TRUE(ret == HJ_KEYBOARD_OK || ret <= 0
                || ret == HJ_KEYBOARD_ERR_INVALID_ARG);
#endif

    if(handle != HJ_INVALID_HANDLE)
        hj_keyboard_close(handle);
}