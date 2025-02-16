#include <gtest/gtest.h>
#include <libcpp/os/application.hpp>

// TEST(application, gettid)
// {
//     ASSERT_EQ(libcpp::application::gettid() > -1, true);
// }

TEST(application, getpid)
{
    ASSERT_EQ(libcpp::application::getpid() > 0, true);
}

TEST(application, getppid)
{
    ASSERT_EQ(libcpp::application::getppid() == 0, true);
}

TEST(application, sleep)
{
    ASSERT_EQ(libcpp::application::sleep(1) == 0, true);
}

TEST(application, daemon)
{

}

TEST(application, raise)
{

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