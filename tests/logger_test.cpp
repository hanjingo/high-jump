#include <gtest/gtest.h>
#include <hj/log/logger.hpp>
#include <filesystem>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <atomic>
#include <algorithm>
#include <spdlog/sinks/base_sink.h>

struct log_record
{
    spdlog::level::level_enum level;
    std::string               message;
};

template <typename Mutex>
class vector_sink : public spdlog::sinks::base_sink<Mutex>
{
  public:
    std::vector<log_record> records;
    bool                    flushed = false;

    std::vector<log_record> get_records()
    {
        std::lock_guard<Mutex> lock(this->mutex_);
        return records;
    }

    bool is_flushed()
    {
        std::lock_guard<Mutex> lock(this->mutex_);
        return flushed;
    }

  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        std::string payload(msg.payload.data(), msg.payload.size());
        records.push_back({msg.level, payload});
    }

    void flush_() override { flushed = true; }
};

TEST(logger, instance)
{
    ASSERT_NE(hj::log::logger::instance(), nullptr);
    auto *inst1 = hj::log::logger::instance();
    auto *inst2 = hj::log::logger::instance();
    ASSERT_EQ(inst1, inst2);
}

TEST(logger, content_level_and_formatting_verification)
{
    hj::log::logger_options opts;
    opts.name  = "verify_test";
    opts.async = false;
    hj::log::logger test_logger(opts);

    test_logger.clear_sink();

    auto  mock_sink     = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_mock_sink = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));
    test_logger.set_level(hj::log::level::trace);

    test_logger.trace("Trace message {}", 1);
    test_logger.debug("Debug message {}", "test");
    test_logger.info("Info value: {:.2f}", 3.14159);
    test_logger.warn("Warning code: {}", 404);
    test_logger.error("Error occurred: {}", "disk full");
    test_logger.critical("Critical failure: {}", true);
    test_logger.flush();

    auto recs = raw_mock_sink->get_records();
    ASSERT_EQ(recs.size(), 6);

    EXPECT_EQ(recs[0].level, spdlog::level::trace);
    EXPECT_EQ(recs[0].message, "Trace message 1");

    EXPECT_EQ(recs[1].level, spdlog::level::debug);
    EXPECT_EQ(recs[1].message, "Debug message test");

    EXPECT_EQ(recs[2].level, spdlog::level::info);
    EXPECT_EQ(recs[2].message, "Info value: 3.14");

    EXPECT_EQ(recs[3].level, spdlog::level::warn);
    EXPECT_EQ(recs[3].message, "Warning code: 404");

    EXPECT_EQ(recs[4].level, spdlog::level::err);
    EXPECT_EQ(recs[4].message, "Error occurred: disk full");

    EXPECT_EQ(recs[5].level, spdlog::level::critical);
    EXPECT_EQ(recs[5].message, "Critical failure: true");

    EXPECT_TRUE(raw_mock_sink->is_flushed());
}

TEST(logger, async_bulk_messages_verification)
{
    hj::log::logger_options opts;
    opts.name       = "async_bulk_test";
    opts.async      = true;
    opts.queue_size = 16384;
    opts.thread_num = 1;
    hj::log::logger test_logger(opts);

    test_logger.clear_sink();

    auto  mock_sink     = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_mock_sink = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));
    test_logger.set_level(hj::log::level::info);

    const int total_messages = 10000;
    for(int i = 0; i < total_messages; ++i)
    {
        test_logger.info("Async message #{}", i);
    }

    test_logger.flush();

    int retry = 0;
    while(raw_mock_sink->get_records().size() < total_messages && retry < 200)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        retry++;
    }

    auto recs = raw_mock_sink->get_records();
    ASSERT_EQ(recs.size(), total_messages);

    EXPECT_EQ(recs[0].message, "Async message #0");
    EXPECT_EQ(recs[9999].message, "Async message #9999");
    EXPECT_EQ(recs[5000].level, spdlog::level::info);
}

TEST(logger, async_overflow_policies_test)
{
    for(auto policy : {hj::log::overflow_policy::block,
                       hj::log::overflow_policy::overrun_oldest,
                       hj::log::overflow_policy::discard_new})
    {
        hj::log::logger_options opts;
        opts.name       = "async_overflow_test";
        opts.async      = true;
        opts.policy     = policy;
        opts.queue_size = 8;
        opts.thread_num = 1;

        hj::log::logger test_logger(opts);
        test_logger.clear_sink();

        auto  mock_sink     = std::make_shared<vector_sink<std::mutex>>();
        auto *raw_mock_sink = mock_sink.get();
        test_logger.add_sink(std::move(mock_sink));
        test_logger.set_level(hj::log::level::info);

        const int total_messages = 500;
        for(int i = 0; i < total_messages; ++i)
        {
            test_logger.info("Flood message {}", i);
        }
        test_logger.flush();

        int retry = 0;
        while(raw_mock_sink->get_records().size() < total_messages
              && policy == hj::log::overflow_policy::block && retry < 100)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            retry++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        auto recs = raw_mock_sink->get_records();

        if(policy == hj::log::overflow_policy::block)
        {
            EXPECT_EQ(recs.size(), total_messages);
            if(!recs.empty())
            {
                EXPECT_EQ(recs.front().message, "Flood message 0");
                EXPECT_EQ(recs.back().message, "Flood message 499");
            }
        } else if(policy == hj::log::overflow_policy::overrun_oldest)
        {
            EXPECT_GT(recs.size(), 0);
            EXPECT_LE(recs.size(), total_messages);
            if(!recs.empty())
            {
                EXPECT_EQ(recs.back().message, "Flood message 499");
            }
        } else if(policy == hj::log::overflow_policy::discard_new)
        {
            EXPECT_GT(recs.size(), 0);
            EXPECT_LT(recs.size(), total_messages);
            if(!recs.empty())
            {
                EXPECT_EQ(recs.front().message, "Flood message 0");
            }
        }
    }
}

TEST(logger, qt_message_literal_safety)
{
#if LOG_QT_SUPPORT
    hj::log::logger_options opts;
    opts.name = "qt_safety_test";
    hj::log::logger test_logger(opts);

    auto  mock_sink     = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_mock_sink = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));

    QString qt_msg = "QML Item created: {id: root, width: 100}";

    ASSERT_NO_THROW({ test_logger.debug("{}", qt_msg.toUtf8().constData()); });

    auto recs = raw_mock_sink->get_records();
    ASSERT_EQ(recs.size(), 1);
    EXPECT_EQ(recs[0].message, "QML Item created: {id: root, width: 100}");
#endif
}

TEST(logger, rigorous_concurrent_structural_stress_test)
{
    hj::log::logger_options opts;
    opts.name        = "stress_logger";
    auto test_logger = std::make_shared<hj::log::logger>(opts);
    test_logger->set_level(hj::log::level::trace);

    std::atomic<bool> running{true};
    const int         duration_ms = 500;

    std::thread writer_thread([&]() {
        long long count = 0;
        while(running.load())
        {
            test_logger->info("Stress test log message #{}", count++);
            test_logger->debug("Debug info counter: {}", count);
        }
    });

    std::thread adder_thread([&]() {
        while(running.load())
        {
            auto temp_sink = std::make_shared<vector_sink<std::mutex>>();
            test_logger->add_sink(std::move(temp_sink));
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });

    std::thread clearer_thread([&]() {
        while(running.load())
        {
            if(test_logger->sink_count() > 10)
            {
                test_logger->clear_sink();
                test_logger->add_sink(hj::log::logger::create_stdout_sink());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::thread level_thread([&]() {
        while(running.load())
        {
            test_logger->set_level(hj::log::level::trace);
            test_logger->set_level(hj::log::level::error);
            test_logger->set_level(hj::log::level::info);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    running.store(false);

    writer_thread.join();
    adder_thread.join();
    clearer_thread.join();
    level_thread.join();

    ASSERT_NO_THROW({
        test_logger->clear_sink();
        test_logger->add_sink(hj::log::logger::create_stdout_sink());
        test_logger->info("Stress test completed successfully.");
        test_logger->flush();
    });
}

TEST(logger, sink_management_and_counts)
{
    auto *log_inst = hj::log::logger::instance();
    log_inst->clear_sink();
    ASSERT_EQ(log_inst->sink_count(), 0);

    auto sink1 = hj::log::logger::create_stdout_sink();
    auto sink2 = hj::log::logger::create_stdout_sink();
    auto raw2  = sink2;

    log_inst->add_sink(std::move(sink1));
    log_inst->add_sink(std::move(sink2));
    ASSERT_EQ(log_inst->sink_count(), 2);

    log_inst->remove_sink(raw2);
    ASSERT_EQ(log_inst->sink_count(), 1);

    log_inst->clear_sink();
    ASSERT_EQ(log_inst->sink_count(), 0);
}

TEST(logger, file_sinks_creation)
{
    std::filesystem::remove_all("test_logs");
    std::filesystem::create_directories("test_logs");
    if(!std::filesystem::exists("test_logs"))
    {
        GTEST_SKIP() << "skip file sink tests: dir creation failed";
    }

    auto rotate_sink =
        hj::log::logger::create_rotate_file_sink("test_logs/rot.log",
                                                 1024,
                                                 2,
                                                 true);
    ASSERT_NE(rotate_sink, nullptr);

    auto daily_sink =
        hj::log::logger::create_daily_file_sink("test_logs/day.log",
                                                0,
                                                0,
                                                false,
                                                2);
    ASSERT_NE(daily_sink, nullptr);
}

TEST(logger_boundary, levels_verification)
{
    hj::log::logger_options opts;
    opts.name  = "level_test";
    opts.async = false;
    hj::log::logger test_logger(opts);
    test_logger.clear_sink();

    auto  mock_sink = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_sink  = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));

    struct LevelTestCase
    {
        hj::log::level set_lvl;
        hj::log::level log_lvl;
        bool           should_pass;
    };

    test_logger.set_level(hj::log::level::off);
    test_logger.info("Should not appear");
    test_logger.flush();
    EXPECT_EQ(raw_sink->get_records().size(), 0);

    test_logger.set_level(hj::log::level::trace);
    test_logger.trace("trace msg");
    test_logger.debug("debug msg");
    test_logger.info("info msg");
    test_logger.warn("warn msg");
    test_logger.error("error msg");
    test_logger.critical("critical msg");
    test_logger.flush();

    auto recs = raw_sink->get_records();
    ASSERT_EQ(recs.size(), 6);
    EXPECT_EQ(recs[0].level, spdlog::level::trace);
    EXPECT_EQ(recs[1].level, spdlog::level::debug);
    EXPECT_EQ(recs[2].level, spdlog::level::info);
    EXPECT_EQ(recs[3].level, spdlog::level::warn);
    EXPECT_EQ(recs[4].level, spdlog::level::err);
    EXPECT_EQ(recs[5].level, spdlog::level::critical);
}

TEST(logger_boundary, formatting_variations)
{
    hj::log::logger_options opts;
    opts.name  = "fmt_test";
    opts.async = false;
    hj::log::logger test_logger(opts);
    test_logger.clear_sink();

    auto  mock_sink = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_sink  = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));
    test_logger.set_level(hj::log::level::trace);

    test_logger.info("hello {}", 42);
    test_logger.info("{} {} {}", 1, 2, 3);

    std::string dynamic_str = "std::string content";
    test_logger.info("{}", dynamic_str);

    test_logger.info("");

    test_logger.raw()->set_error_handler(
        [](const std::string &msg) { throw std::runtime_error(msg); });

    EXPECT_ANY_THROW({
        test_logger.info("{");
        test_logger.flush();
    });

    test_logger.flush();
    auto recs = raw_sink->get_records();

    ASSERT_GE(recs.size(), 4);
    EXPECT_EQ(recs[0].message, "hello 42");
    EXPECT_EQ(recs[1].message, "1 2 3");
    EXPECT_EQ(recs[2].message, "std::string content");
    EXPECT_EQ(recs[3].message, "");
}

TEST(logger_boundary, large_messages)
{
    hj::log::logger_options opts;
    opts.name  = "large_msg_test";
    opts.async = false;
    hj::log::logger test_logger(opts);
    test_logger.clear_sink();

    auto  mock_sink = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_sink  = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));
    test_logger.set_level(hj::log::level::info);

    std::string msg_1mb(1024 * 1024, 'A');
    std::string msg_10mb(10 * 1024 * 1024, 'B');

    ASSERT_NO_THROW({
        test_logger.info("{}", msg_1mb);
        test_logger.flush();
    });

    ASSERT_NO_THROW({
        test_logger.info("{}", msg_10mb);
        test_logger.flush();
    });

    auto recs = raw_sink->get_records();
    ASSERT_EQ(recs.size(), 2);
    EXPECT_EQ(recs[0].message.size(), 1024 * 1024);
    EXPECT_EQ(recs[1].message.size(), 10 * 1024 * 1024);
}

class logger_multithread_param_test : public ::testing::TestWithParam<int>
{
};

TEST_P(logger_multithread_param_test, thread_scaling_stress)
{
    int thread_count = GetParam();

    hj::log::logger_options opts;
    opts.name       = "mt_test";
    opts.async      = true;
    opts.queue_size = 32768;
    opts.thread_num = 2;
    hj::log::logger test_logger(opts);
    test_logger.clear_sink();

    auto  mock_sink = std::make_shared<vector_sink<std::mutex>>();
    auto *raw_sink  = mock_sink.get();
    test_logger.add_sink(std::move(mock_sink));
    test_logger.set_level(hj::log::level::info);

    std::atomic<bool>        start_flag{false};
    std::vector<std::thread> workers;
    const int                messages_per_thread = 200;

    for(int t = 0; t < thread_count; ++t)
    {
        workers.emplace_back(
            [&test_logger, &start_flag, t, messages_per_thread]() {
                while(!start_flag.load())
                {
                    std::this_thread::yield();
                }
                for(int i = 0; i < messages_per_thread; ++i)
                {
                    test_logger.info("Thread {} msg {}", t, i);
                }
            });
    }

    start_flag.store(true);
    for(auto &w : workers)
    {
        w.join();
    }

    test_logger.flush();
    int retry          = 0;
    int expected_total = thread_count * messages_per_thread;
    while(raw_sink->get_records().size() < expected_total && retry < 300)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        retry++;
    }

    EXPECT_EQ(raw_sink->get_records().size(), expected_total);
}

INSTANTIATE_TEST_SUITE_P(ThreadCounts,
                         logger_multithread_param_test,
                         ::testing::Values(1, 2, 4, 8, 16, 32));

TEST(logger_boundary, async_discard_new_policy_and_destruction)
{
    {
        hj::log::logger_options opts;
        opts.name       = "discard_test";
        opts.async      = true;
        opts.policy     = hj::log::overflow_policy::discard_new;
        opts.queue_size = 4;
        opts.thread_num = 1;

        hj::log::logger test_logger(opts);
        test_logger.clear_sink();
        auto mock_sink = std::make_shared<vector_sink<std::mutex>>();
        test_logger.add_sink(std::move(mock_sink));

        for(int i = 0; i < 100; ++i)
        {
            test_logger.info("Discard test {}", i);
        }
        test_logger.flush();
    }

    {
        std::unique_ptr<hj::log::logger> test_logger =
            std::make_unique<hj::log::logger>([]() {
                hj::log::logger_options opts;
                opts.name       = "dtor_test";
                opts.async      = true;
                opts.queue_size = 1024;
                return opts;
            }());

        for(int i = 0; i < 50; ++i)
        {
            test_logger->info("Destruction flood {}", i);
        }
        ASSERT_NO_THROW({ test_logger.reset(); });
    }
}

TEST(logger_boundary, sink_comprehensive_management)
{
    hj::log::logger_options opts;
    opts.name = "sink_boundary_test";
    hj::log::logger test_logger(opts);

    test_logger.clear_sink();
    EXPECT_EQ(test_logger.sink_count(), 0);

    auto sink1 = hj::log::logger::create_stdout_sink();
    auto sink2 = hj::log::logger::create_stdout_sink();
    auto raw1  = sink1;

    test_logger.add_sink(std::move(sink1));
    test_logger.add_sink(std::move(sink2));
    EXPECT_EQ(test_logger.sink_count(), 2);

    test_logger.remove_sink(raw1);
    EXPECT_EQ(test_logger.sink_count(), 1);

    EXPECT_NO_THROW({ test_logger.remove_sink(nullptr); });

    EXPECT_THROW({ test_logger.add_sink(nullptr); }, std::invalid_argument);

    test_logger.clear_sink();
    EXPECT_EQ(test_logger.sink_count(), 0);
}

TEST(logger_boundary, file_sinks_advanced_options)
{
    std::string test_dir = "test_logs_advanced";
    std::filesystem::remove_all(test_dir);
    std::filesystem::create_directories(test_dir);

    if(!std::filesystem::exists(test_dir))
    {
        GTEST_SKIP()
            << "Skipping file sink advanced tests: directory creation failed.";
    }

    {
        std::string rot_path = test_dir + "/rot_open.log";
        {
            {
                auto init_sink =
                    hj::log::logger::create_rotate_file_sink(rot_path,
                                                             100,
                                                             3,
                                                             false);
            }

            auto rotate_sink_on_open =
                hj::log::logger::create_rotate_file_sink(rot_path,
                                                         100,
                                                         3,
                                                         true);
            ASSERT_NE(rotate_sink_on_open, nullptr);
        }

        std::string daily_path = test_dir + "/daily_test.log";
        auto        daily_sink_custom =
            hj::log::logger::create_daily_file_sink(daily_path,
                                                    23,
                                                    59,
                                                    true,
                                                    5);
        ASSERT_NE(daily_sink_custom, nullptr);
    }

    std::filesystem::remove_all(test_dir);
}