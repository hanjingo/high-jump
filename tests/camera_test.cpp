#include <gtest/gtest.h>
#include <libcpp/hardware/camera.h>
#include <opencv2/opencv.hpp>

using libcpp::camera;

TEST(CameraTest, OpenAndRelease) {
    camera cam;
    ASSERT_TRUE(cam.open(0) || cam.open("/dev/video0"));
    ASSERT_TRUE(cam.is_opened());
    cam.release();
    ASSERT_FALSE(cam.is_opened());
}

TEST(CameraTest, ReadFrame) {
    camera cam;
    if (!(cam.open(0) || cam.open("/dev/video0"))) {
        GTEST_SKIP() << "No camera available";
    }
    ASSERT_TRUE(cam.is_opened());
    cv::Mat frame;
    ASSERT_TRUE(cam.read(frame));
    ASSERT_FALSE(frame.empty());
    cam.release();
}

TEST(CameraTest, SetAndGetProperty) {
    camera cam;
    if (!(cam.open(0) || cam.open("/dev/video0"))) {
        GTEST_SKIP() << "No camera available";
    }
    ASSERT_TRUE(cam.is_opened());
    double width = cam.get(cv::CAP_PROP_FRAME_WIDTH);
    ASSERT_GT(width, 0);
    cam.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cam.release();
}

