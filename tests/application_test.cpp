#include <gtest/gtest.h>
#include <libcpp/os/application.hpp>

TEST(application, sleep)
{
    libcpp::application::sleep(1);
}

TEST(application, msleep)
{
    libcpp::application::msleep(1);
}