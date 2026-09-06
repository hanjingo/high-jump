/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <hj/os/signal.hpp>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#define TEST_SIG1 SIGINT
#define TEST_SIG2 SIGTERM
#define TEST_SIG3 SIGABRT
#else
#define TEST_SIG1 SIGUSR1
#define TEST_SIG2 SIGUSR2
#define TEST_SIG3 SIGHUP
#endif

class SignalTest : public ::testing::Test
{
  public:
    static void do_clear_queue()
    {
        hj::sighandler::instance()._clear_queue_internal();
    }

  protected:
    void SetUp() override
    {
        hj::sighandler::instance().sigunregister(
            {TEST_SIG1, TEST_SIG2, TEST_SIG3});
        do_clear_queue();
        hj::sighandler::instance().set_overflow_policy(
            hj::sig_overflow_policy::reject);
    }

    void TearDown() override
    {
        hj::sighandler::instance().sigunregister(
            {TEST_SIG1, TEST_SIG2, TEST_SIG3});
        do_clear_queue();
    }
};

TEST_F(SignalTest, Unregister)
{
    using hj::sighandler;
    int count = 0;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });
    sighandler::instance().sigunregister(TEST_SIG1);

    sighandler::instance().signotify(TEST_SIG1);
    size_t processed = sighandler::instance().poll();

    EXPECT_EQ(processed, 0);
    EXPECT_EQ(count, 0);
    EXPECT_FALSE(sighandler::instance().is_registered(TEST_SIG1));
}

TEST_F(SignalTest, PollReturnValueAndMaxEvents)
{
    using hj::sighandler;
    std::atomic<int> count{0};

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });

    for(int i = 0; i < 10; ++i)
    {
        sighandler::instance().signotify(TEST_SIG1);
    }

    size_t processed_batch1 = sighandler::instance().poll(4);
    EXPECT_EQ(processed_batch1, 4);
    EXPECT_EQ(count.load(), 4);

    size_t processed_batch2 = sighandler::instance().poll(4);
    EXPECT_EQ(processed_batch2, 4);
    EXPECT_EQ(count.load(), 8);

    size_t processed_batch3 = sighandler::instance().poll();
    EXPECT_EQ(processed_batch3, 2);
    EXPECT_EQ(count.load(), 10);

    size_t processed_batch4 = sighandler::instance().poll();
    EXPECT_EQ(processed_batch4, 0);
}

TEST_F(SignalTest, OneShotBehavior)
{
    using hj::sighandler;
    int count = 0;

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; }, true);

    sighandler::instance().signotify(TEST_SIG1);
    EXPECT_EQ(sighandler::instance().poll(), 1);
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(sighandler::instance().is_registered(TEST_SIG1));

    sighandler::instance().signotify(TEST_SIG1);
    EXPECT_EQ(sighandler::instance().poll(), 0);
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
        sighandler::instance().signotify(TEST_SIG1);
    }

    EXPECT_EQ(sighandler::instance().poll(), BURST_SIZE);
    EXPECT_EQ(count.load(), BURST_SIZE);
}

TEST_F(SignalTest, ConcurrentRaiseAndNotify)
{
    using hj::sighandler;
    std::atomic<int> count1{0};
    std::atomic<int> count2{0};

    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count1++; });
    sighandler::instance().sigcatch(TEST_SIG2, [&](int) { count2++; });

    constexpr int RAISES_PER_THREAD = 30;
    auto          notify_func       = [RAISES_PER_THREAD](int sig) {
        for(int i = 0; i < RAISES_PER_THREAD; ++i)
        {
            sighandler::instance().signotify(sig);
            std::this_thread::yield();
        }
    };

    std::thread t1(notify_func, TEST_SIG1);
    std::thread t2(notify_func, TEST_SIG2);
    std::thread t3(notify_func, TEST_SIG1);

    t1.join();
    t2.join();
    t3.join();

    size_t total = sighandler::instance().poll();
    EXPECT_EQ(total, RAISES_PER_THREAD * 3);
    EXPECT_EQ(count1.load(), RAISES_PER_THREAD * 2);
    EXPECT_EQ(count2.load(), RAISES_PER_THREAD);
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

    sighandler::instance().signotify(TEST_SIG1);

    worker.join();

    EXPECT_NE(callback_thread_id, std::thread::id());
    EXPECT_EQ(callback_thread_id, poll_thread_id);
    EXPECT_NE(callback_thread_id, std::this_thread::get_id());
}

TEST_F(SignalTest, QueueOverflowAndPolicy)
{
    using hj::sig_overflow_policy;
    using hj::sighandler;

    std::atomic<int> count{0};
    sighandler::instance().sigcatch(TEST_SIG1, [&](int) { count++; });

    constexpr int OVERFLOW_SIZE = 150;

    sighandler::instance().set_overflow_policy(sig_overflow_policy::reject);
    sighandler::instance().reset_dropped_count();
    SignalTest::do_clear_queue();
    count.store(0);

    for(int i = 0; i < OVERFLOW_SIZE; ++i)
    {
        sighandler::instance().signotify(TEST_SIG1);
    }

    EXPECT_EQ(sighandler::instance().dropped_count(), 50);

    size_t processed = sighandler::instance().poll();
    EXPECT_EQ(processed,
              static_cast<size_t>(sighandler::MAX_PENDING_PER_SIGNAL));
    EXPECT_EQ(count.load(),
              static_cast<int>(sighandler::MAX_PENDING_PER_SIGNAL));

    sighandler::instance().set_overflow_policy(sig_overflow_policy::overwrite);
    sighandler::instance().reset_dropped_count();
    SignalTest::do_clear_queue();
    count.store(0);

    for(int i = 0; i < OVERFLOW_SIZE; ++i)
    {
        sighandler::instance().signotify(TEST_SIG1);
    }

    EXPECT_EQ(sighandler::instance().dropped_count(), 50);

    processed = sighandler::instance().poll();
    EXPECT_EQ(processed,
              static_cast<size_t>(sighandler::MAX_PENDING_PER_SIGNAL));
    EXPECT_EQ(count.load(),
              static_cast<int>(sighandler::MAX_PENDING_PER_SIGNAL));
}