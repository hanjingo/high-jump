#include <gtest/gtest.h>

#define HJ_MOUSE_IMPL
#include <hj/hardware/mouse.h>

TEST(mouse, enumerate_mouse)
{
    hj_mouse_info_t infos[8] = {0};
    int             n;
    hj_mouse_enumerate(infos, 8, &n);
    EXPECT_GE(n, 0);
    if(n == 0)
    {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    for(int i = 0; i < n; ++i)
    {
        EXPECT_GT(strlen(infos[i].manufacturer), 0);
        EXPECT_GT(strlen(infos[i].product), 0);
    }
}

TEST(mouse, open_close_mouse)
{
    hj_mouse_info_t infos[4] = {0};
    int             n        = 0;
    hj_mouse_enumerate(infos, 4, &n);
    if(n == 0)
    {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }

    intptr_t handle = hj_mouse_open(infos[0].device_path);
    if(handle <= 0)
    {
        GTEST_SKIP() << "Permission denied or handle invalid, skipping.";
    }

    EXPECT_GT(handle, 0);
    hj_mouse_close(handle);
}
TEST(mouse, read_mouse_event)
{
    hj_mouse_info_t infos[2] = {0};
    int             n        = 0;
    hj_mouse_enumerate(infos, 2, &n);
    if(n == 0)
    {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }
    intptr_t handle = hj_mouse_open(infos[0].device_path);
    if(handle < 0)
    {
        GTEST_SKIP() << "Unable to open device handle, skipping read test.";
    }

    hj_mouse_event_t event = {0};
    hj_mouse_err_t   ret   = hj_mouse_read_event(handle, &event);
    EXPECT_TRUE(ret == HJ_MOUSE_SUCCESS || ret == HJ_MOUSE_ERROR_NO_DATA);
    hj_mouse_close(handle);
}

TEST(mouse, set_mouse_param)
{
    hj_mouse_info_t infos[1] = {0};
    int             n        = 0;
    hj_mouse_enumerate(infos, 1, &n);
    if(n == 0)
    {
        GTEST_SKIP() << "No mouse device found, skipping test.";
    }

    intptr_t handle = hj_mouse_open(infos[0].device_path);
    if(handle <= 0)
    {
        GTEST_SKIP() << "Permission denied or handle invalid, skipping.";
    }

    hj_mouse_err_t ret = hj_mouse_set_param(handle, 10);
    EXPECT_TRUE(ret == HJ_MOUSE_SUCCESS || ret == HJ_MOUSE_ERROR_NOT_SUPPORTED);
    hj_mouse_close(handle);
}

#ifdef __linux__

TEST(mouse_logic, parse_motion_event)
{
    struct input_event raw_ev = {0};
    hj_mouse_event_t   out_ev = {0};

    raw_ev.time.tv_sec  = 12345;
    raw_ev.time.tv_usec = 6789;
    raw_ev.type         = EV_REL;
    raw_ev.code         = REL_X;
    raw_ev.value        = 15;

    hj_mouse_err_t ret = hj_mouse_parse_input_event(&raw_ev, &out_ev);
    EXPECT_EQ(ret, HJ_MOUSE_SUCCESS);
    EXPECT_EQ(out_ev.type, HJ_MOUSE_EVENT_MOTION);
    EXPECT_EQ(out_ev.dx, 15);
    EXPECT_EQ(out_ev.timestamp_us, 12345006789ULL);
}

TEST(mouse_logic, parse_button_event)
{
    struct input_event raw_ev = {0};
    hj_mouse_event_t   out_ev = {0};

    raw_ev.type  = EV_KEY;
    raw_ev.code  = BTN_RIGHT;
    raw_ev.value = 1;

    hj_mouse_err_t ret = hj_mouse_parse_input_event(&raw_ev, &out_ev);
    EXPECT_EQ(ret, HJ_MOUSE_SUCCESS);
    EXPECT_EQ(out_ev.type, HJ_MOUSE_EVENT_BUTTON);
    EXPECT_EQ(out_ev.button, HJ_MOUSE_BTN_RIGHT);
    EXPECT_EQ(out_ev.pressed, 1);
}

TEST(mouse_logic, parse_wheel_event)
{
    struct input_event raw_ev = {0};
    hj_mouse_event_t   out_ev = {0};

    raw_ev.type  = EV_REL;
    raw_ev.code  = REL_WHEEL;
    raw_ev.value = -1;

    hj_mouse_err_t ret = hj_mouse_parse_input_event(&raw_ev, &out_ev);
    EXPECT_EQ(ret, HJ_MOUSE_SUCCESS);
    EXPECT_EQ(out_ev.type, HJ_MOUSE_EVENT_WHEEL);
    EXPECT_EQ(out_ev.wheel_delta, -1);
}

TEST(mouse_logic, ignore_unknown_event)
{
    struct input_event raw_ev = {0};
    hj_mouse_event_t   out_ev = {0};

    raw_ev.type = EV_MSC;
    raw_ev.code = MSC_SCAN;

    hj_mouse_err_t ret = hj_mouse_parse_input_event(&raw_ev, &out_ev);
    EXPECT_EQ(ret, HJ_MOUSE_ERROR_NO_DATA);
}

#endif