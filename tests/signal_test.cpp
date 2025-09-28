#include <gtest/gtest.h>
#include <hj/os/signal.hpp>

static int signal_num = 0;

void handler1(int sig)
{
    (void) sig;
    signal_num += 1;
}

void handler2(int sig)
{
    (void) sig;
    signal_num += 2;
}

TEST(signal, sigcatch)
{
    hj::_sigcatch(SIGABRT, std::bind(handler1, std::placeholders::_1), true);

    hj::sigcatch({SIGILL, SIGTERM},
                 std::bind(handler2, std::placeholders::_1),
                 true);

    hj::sig_raise(SIGABRT);
    ASSERT_EQ(signal_num, 1);
    hj::sig_raise(SIGABRT);
    ASSERT_EQ(signal_num, 1);

    hj::sig_raise(SIGILL);
    ASSERT_EQ(signal_num, 3);
    hj::sig_raise(SIGTERM);
    ASSERT_EQ(signal_num, 5);

    hj::sig_raise(SIGILL, SIGTERM);
    ASSERT_EQ(signal_num, 5);
}