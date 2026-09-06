#include <gtest/gtest.h>
#include <hj/os/signal.hpp>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#define TEST_SIG1 SIGINT
#define TEST_SIG2 SIGFPE
#define TEST_SIG3 SIGSEGV
#else
#define TEST_SIG1 SIGUSR1
#define TEST_SIG2 SIGUSR2
#define TEST_SIG3 SIGHUP
#endif

class SignalTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        hj::sighandler::instance().sigunregister(
            {TEST_SIG1, TEST_SIG2, TEST_SIG3});
        hj::sighandler::instance().clear_queue();
        hj::sighandler::instance().set_overflow_policy(
            hj::sig_overflow_policy::reject);
    }

    void TearDown() override
    {
        hj::sighandler::instance().sigunregister(
            {TEST_SIG1, TEST_SIG2, TEST_SIG3});
        hj::sighandler::instance().clear_queue();
    }
};

TEST_F(SignalTest, Unregister)
{
    using hj::sighandler;
    int count = 0;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });
    sighandler::instance().sigunregister(TEST_SIG1);

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();

    EXPECT_EQ(count, 0);
    EXPECT_FALSE(sighandler::instance().is_registered(TEST_SIG1));
}

TEST_F(SignalTest, OneShotBehavior)
{
    using hj::sighandler;
    int count = 0;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; }, true);

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(sighandler::instance().is_registered(TEST_SIG1));

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();
    EXPECT_EQ(count, 1);
}

TEST_F(SignalTest, QueueBurst)
{
    using hj::sighandler;
    std::atomic<int> count{0};

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });

    constexpr int BURST_SIZE = 100;
    for(int i = 0; i < BURST_SIZE; ++i)
    {
        sighandler::instance().sigraise(TEST_SIG1);
    }

    sighandler::instance().poll();
    EXPECT_EQ(count.load(), BURST_SIZE);
}

TEST_F(SignalTest, QueueOverflowAndPolicy)
{
    using hj::sig_overflow_policy;
    using hj::sighandler;

    std::atomic<int> count{0};
    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });

    sighandler::instance().set_overflow_policy(sig_overflow_policy::reject);
    constexpr int OVERFLOW_SIZE = 150;

    for(int i = 0; i < OVERFLOW_SIZE; ++i)
    {
        sighandler::instance().sigraise(TEST_SIG1);
    }

    EXPECT_GT(sighandler::instance().dropped_count(), 0);

    sighandler::instance().poll();
    EXPECT_LE(count.load(), static_cast<int>(sighandler::RING_BUFFER_SIZE));
}

TEST_F(SignalTest, ConcurrentRaise)
{
    using hj::sighandler;
    std::atomic<int> count1{0};
    std::atomic<int> count2{0};

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count1++; });
    sighandler::instance().sigcatch(TEST_SIG2, [&](int) { count2++; });

    constexpr int RAISES_PER_THREAD = 30;
    auto          raise_func        = [RAISES_PER_THREAD](int sig) {
        for(int i = 0; i < RAISES_PER_THREAD; ++i)
        {
            sighandler::instance().sigraise(sig);
            std::this_thread::yield();
        }
    };

    std::thread t1(raise_func, TEST_SIG1);
    std::thread t2(raise_func, TEST_SIG2);
    std::thread t3(raise_func, TEST_SIG1);

    t1.join();
    t2.join();
    t3.join();

    sighandler::instance().poll();

    EXPECT_EQ(count1.load(), RAISES_PER_THREAD * 2);
    EXPECT_EQ(count2.load(), RAISES_PER_THREAD);
}

TEST_F(SignalTest, ConcurrentRegisterUnregister)
{
    using hj::sighandler;
    std::atomic<bool> stop{false};

    std::thread reg_thread([&]() {
        while(!stop)
        {
            sighandler::instance().sigcatch(TEST_SIG1, [](int) {});
            std::this_thread::yield();
        }
    });

    std::thread unreg_thread([&]() {
        while(!stop)
        {
            sighandler::instance().sigunregister(TEST_SIG1);
            std::this_thread::yield();
        }
    });

    std::thread raise_thread([&]() {
        while(!stop)
        {
            sighandler::instance().sigraise(TEST_SIG1);
            sighandler::instance().poll();
            std::this_thread::yield();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop = true;

    reg_thread.join();
    unreg_thread.join();
    raise_thread.join();
}

TEST_F(SignalTest, CallbackReentrancy)
{
    using hj::sighandler;

    bool cb1_executed = false;
    bool cb2_executed = false;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) {
        cb1_executed = true;
        sighandler::instance().sigunregister(TEST_SIG1);
        sighandler::instance().sigcatch(TEST_SIG2,
                                        [&](int) { cb2_executed = true; });
    });

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().poll();

    EXPECT_TRUE(cb1_executed);
    EXPECT_FALSE(sighandler::instance().is_registered(TEST_SIG1));
    EXPECT_TRUE(sighandler::instance().is_registered(TEST_SIG2));

    sighandler::instance().sigraise(TEST_SIG2);
    sighandler::instance().poll();
    EXPECT_TRUE(cb2_executed);
}

TEST_F(SignalTest, CallbackThrowsException)
{
    using hj::sighandler;

    int normal_count = 0;

    sighandler::instance().sigcatch(TEST_SIG1, [](int) {
        throw std::runtime_error("Exception in signal handler callback");
    });
    sighandler::instance().sigcatch(TEST_SIG2, [&](int) { normal_count++; });

    sighandler::instance().sigraise(TEST_SIG1);
    sighandler::instance().sigraise(TEST_SIG2);

    EXPECT_NO_THROW(sighandler::instance().poll());
    EXPECT_EQ(normal_count, 1);
}

TEST_F(SignalTest, InterruptPolicyCatch)
{
    using hj::sig_interrupt_policy;
    using hj::sighandler;

    sighandler::instance()
        .sigcatch(TEST_SIG1, [](int) {}, false, sig_interrupt_policy::none);
    EXPECT_TRUE(sighandler::instance().is_registered(TEST_SIG1));

    sighandler::instance()
        .sigcatch(TEST_SIG2, [](int) {}, false, sig_interrupt_policy::restart);
    EXPECT_TRUE(sighandler::instance().is_registered(TEST_SIG2));
}

TEST_F(SignalTest, ExecutionContextVerification)
{
    using hj::sighandler;

    std::thread::id poll_thread_id;
    std::thread::id callback_thread_id;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) {
        callback_thread_id = std::this_thread::get_id();
    });

    std::thread worker([&]() {
        poll_thread_id = std::this_thread::get_id();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sighandler::instance().poll();
    });

    sighandler::instance().sigraise(TEST_SIG1);

    worker.join();

    EXPECT_NE(callback_thread_id, std::thread::id());
    EXPECT_EQ(callback_thread_id, poll_thread_id);
    EXPECT_NE(callback_thread_id, std::this_thread::get_id());
}