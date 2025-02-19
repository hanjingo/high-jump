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

// TEST(application, sig_reg)
// {
//     auto ret = libcpp::application::sig_reg(
//         SIGABRT, 
//         [](int sig, libcpp::application::sig_info* info, void*){
//             ASSERT_EQ(sig == SIGABRT, true);
//         }, 
//         true);

//     libcpp::application::sig_raise(SIGABRT);
// }

// TEST(application, sig_unreg)
// {
//     libcpp::application::sig_unreg(SIGABRT);

//     auto ret = libcpp::application::sig_reg(
//         SIGABRT, 
//         [](int sig, libcpp::application::sig_info* info, void*){
//             ASSERT_EQ(sig == SIGABRT, true);
//         }, 
//         true);

//     libcpp::application::sig_raise(SIGABRT);
// }