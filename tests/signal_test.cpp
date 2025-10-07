#include <gtest/gtest.h>
#include <hj/os/signal.hpp>

#if defined(_WIN32) || defined(_WIN64)
#define TEST_SIG1 SIGINT
#define TEST_SIG2 SIGFPE
#else
#define TEST_SIG1 SIGUSR1
#define TEST_SIG2 SIGUSR2
#endif

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

TEST(signal, sigcatch_and_poll)
{
    using hj::sighandler;

    signal_num = 0;

    sighandler::instance().sigcatch(TEST_SIG1, handler1, false);
    sighandler::instance().sigcatch({TEST_SIG2}, handler2, false);

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();
    ASSERT_EQ(signal_num, 1);

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();
    ASSERT_EQ(signal_num, 2);

    sighandler::instance().sigraise(TEST_SIG2);
    sighandler::instance().poll();
    ASSERT_EQ(signal_num, 4);

    sighandler::instance().sigraise(TEST_SIG2);
    sighandler::instance().poll();
    ASSERT_EQ(signal_num, 6);
}

TEST(signal, sigcatch_status)
{
    using hj::sighandler;
    signal_num = 0;

    sighandler::instance().sigcatch(TEST_SIG1, handler1, false);
    ASSERT_TRUE(sighandler::instance().is_registered(TEST_SIG1));
    ASSERT_FALSE(sighandler::instance().is_one_shoot(TEST_SIG1));

    sighandler::instance().sigcatch(TEST_SIG2, handler2, true);
    ASSERT_TRUE(sighandler::instance().is_registered(TEST_SIG2));
    ASSERT_TRUE(sighandler::instance().is_one_shoot(TEST_SIG2));
}