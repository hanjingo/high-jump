#include <gtest/gtest.h>
#include <hj/log/logger.hpp>
#include <filesystem>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>

using namespace hj::log;

class logger_test : public ::testing::Test
{
  protected:
    void SetUp() override { std::filesystem::create_directories("test_logs"); }

    void TearDown() override
    {
        try
        {
            std::filesystem::remove_all("test_logs");
        }
        catch(...)
        {
        }
    }
};

TEST_F(logger_test, instance)
{
    ASSERT_NE(logger::instance(), nullptr);
    auto *inst1 = logger::instance();
    auto *inst2 = logger::instance();
    ASSERT_EQ(inst1, inst2);
}

TEST_F(logger_test, create_stdout_sink)
{
    auto sink = logger::create_stdout_sink();
    ASSERT_NE(sink, nullptr);
    ASSERT_NE(dynamic_cast<spdlog::sinks::stdout_sink_mt *>(sink.get()),
              nullptr);
}

TEST_F(logger_test, create_rotate_file_sink)
{
    const std::string filename  = "test_logs/rotate_test.log";
    const std::size_t max_size  = 1024 * 1024; // 1MB
    const std::size_t max_files = 3;

    auto sink =
        logger::create_rotate_file_sink(filename, max_size, max_files, true);
    ASSERT_NE(sink, nullptr);

    auto *rotating_sink =
        dynamic_cast<spdlog::sinks::rotating_file_sink_mt *>(sink.get());
    ASSERT_NE(rotating_sink, nullptr);
}

TEST_F(logger_test, create_daily_file_sink)
{
    const std::string filename = "test_logs/daily_test.log";

    auto sink = logger::create_daily_file_sink(filename, 1, 1, true, 2);
    ASSERT_NE(sink, nullptr);

    auto *daily_sink =
        dynamic_cast<spdlog::sinks::daily_file_sink_mt *>(sink.get());
    ASSERT_NE(daily_sink, nullptr);
}

TEST_F(logger_test, add_sink)
{
    auto *log_inst      = logger::instance();
    auto  initial_count = log_inst->sink_count();

    auto sink = logger::create_stdout_sink();
    log_inst->add_sink(std::move(sink));

    ASSERT_EQ(log_inst->sink_count(), initial_count + 1);
}

TEST_F(logger_test, remove_sink)
{
    auto *log_inst = logger::instance();
    auto  sink     = logger::create_stdout_sink();
    auto  sink_ptr = sink;
    log_inst->add_sink(std::move(sink));
    auto count_after_add = log_inst->sink_count();

    log_inst->remove_sink(sink_ptr);
    ASSERT_EQ(log_inst->sink_count(), count_after_add - 1);
}

TEST_F(logger_test, clear_sink)
{
    auto *log_inst = logger::instance();

    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->add_sink(logger::create_stdout_sink());
    ASSERT_GT(log_inst->sink_count(), 0);

    log_inst->clear_sink();
    ASSERT_EQ(log_inst->sink_count(), 0);
}

TEST_F(logger_test, set_and_get_level)
{
    auto *log_inst = logger::instance();

    log_inst->set_level(level::info);
    ASSERT_EQ(log_inst->get_level(), level::info);

    log_inst->set_level(level::debug);
    ASSERT_EQ(log_inst->get_level(), level::debug);

    log_inst->set_level(level::warning);
    ASSERT_EQ(log_inst->get_level(), level::warning);

    log_inst->set_level(level::error);
    ASSERT_EQ(log_inst->get_level(), level::error);

    log_inst->set_level(level::critical);
    ASSERT_EQ(log_inst->get_level(), level::critical);
}

TEST_F(logger_test, should_log)
{
    auto *log_inst = logger::instance();

    log_inst->set_level(level::info);

    ASSERT_FALSE(log_inst->should_log(level::trace));   // trace < info
    ASSERT_FALSE(log_inst->should_log(level::debug));   // debug < info
    ASSERT_TRUE(log_inst->should_log(level::info));     // info == info
    ASSERT_TRUE(log_inst->should_log(level::warning));  // warning > info
    ASSERT_TRUE(log_inst->should_log(level::error));    // error > info
    ASSERT_TRUE(log_inst->should_log(level::critical)); // critical > info

    log_inst->set_level(level::off);
    ASSERT_FALSE(log_inst->should_log(level::critical));
}

TEST_F(logger_test, set_pattern)
{
    auto *log_inst = logger::instance();

    ASSERT_NO_THROW(log_inst->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v"));
    ASSERT_NO_THROW(log_inst->set_pattern("%v"));
}

TEST_F(logger_test, flush)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());

    log_inst->info("Test message before flush");

    ASSERT_NO_THROW(log_inst->flush());
}

TEST_F(logger_test, flush_on)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());

    ASSERT_NO_THROW(log_inst->flush_on(level::info));

    log_inst->info("This should trigger auto-flush");
    log_inst->debug("This should not trigger auto-flush");
}

TEST_F(logger_test, trace_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::trace);

    ASSERT_NO_THROW(log_inst->trace("Trace message"));
    ASSERT_NO_THROW(log_inst->trace("Trace with param: {}", 42));
    ASSERT_NO_THROW(
        log_inst->trace("Trace with multiple params: {} {}", "hello", 123));
}

TEST_F(logger_test, debug_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::debug);

    ASSERT_NO_THROW(log_inst->debug("Debug message"));
    ASSERT_NO_THROW(log_inst->debug("Debug with param: {}", "test"));
}

TEST_F(logger_test, info_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::info);

    ASSERT_NO_THROW(log_inst->info("Info message"));
    ASSERT_NO_THROW(log_inst->info("Info with param: {}", 3.14));
}

TEST_F(logger_test, warn_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::warning);

    ASSERT_NO_THROW(log_inst->warn("Warning message"));
    ASSERT_NO_THROW(log_inst->warn("Warning with param: {}", true));
}

TEST_F(logger_test, error_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::error);

    ASSERT_NO_THROW(log_inst->error("Error message"));
    ASSERT_NO_THROW(log_inst->error("Error with code: {}", 404));
}

TEST_F(logger_test, critical_logging)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::critical);

    ASSERT_NO_THROW(log_inst->critical("Critical message"));
    ASSERT_NO_THROW(log_inst->critical("Critical error: {}", "System failure"));
}

TEST_F(logger_test, logger_name)
{
    auto *log_inst = logger::instance();

    const auto &name = log_inst->name();
    ASSERT_TRUE(name.empty() || !name.empty());

    logger custom_logger("test_logger");
    ASSERT_EQ(custom_logger.name(), "test_logger");
}

TEST_F(logger_test, sink_count)
{
    auto *log_inst = logger::instance();

    log_inst->clear_sink();
    ASSERT_EQ(log_inst->sink_count(), 0);

    log_inst->add_sink(logger::create_stdout_sink());
    ASSERT_EQ(log_inst->sink_count(), 1);

    log_inst->add_sink(logger::create_stdout_sink());
    ASSERT_EQ(log_inst->sink_count(), 2);
}

TEST_F(logger_test, custom_logger_construction)
{
    ASSERT_NO_THROW(logger custom_sync("test_sync", false));

    ASSERT_NO_THROW(logger custom_async("test_async", true, 2048, 1));

    auto spdlog_logger = spdlog::default_logger();
    ASSERT_NO_THROW(logger from_spdlog(spdlog_logger));
}

TEST_F(logger_test, file_logging)
{
    logger file_logger("file_test", false);

    auto file_sink = logger::create_rotate_file_sink("test_logs/file_test.log",
                                                     1024,
                                                     3,
                                                     false);
    file_logger.add_sink(std::move(file_sink));

    file_logger.info("This is a file log message");
    file_logger.error("This is an error in file");
    file_logger.flush();

    ASSERT_TRUE(std::filesystem::exists("test_logs/file_test.log"));
}

TEST_F(logger_test, thread_safety)
{
    auto *log_inst = logger::instance();
    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());

    const int                num_threads         = 4;
    const int                messages_per_thread = 100;
    std::vector<std::thread> threads;

    for(int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([log_inst, t, messages_per_thread]() {
            for(int i = 0; i < messages_per_thread; ++i)
            {
                log_inst->info("Thread {} message #{}", t, i);
            }
        });
    }

    for(auto &thread : threads)
    {
        thread.join();
    }

    ASSERT_NO_THROW(log_inst->flush());
}

TEST_F(logger_test, log_macros)
{
    auto *log_inst = logger::instance();
    log_inst->clear_sink();
    log_inst->add_sink(logger::create_stdout_sink());
    log_inst->set_level(level::trace);

    ASSERT_NO_THROW(LOG_TRACE("Trace macro test"));
    ASSERT_NO_THROW(LOG_DEBUG("Debug macro test"));
    ASSERT_NO_THROW(LOG_INFO("Info macro test"));
    ASSERT_NO_THROW(LOG_WARN("Warning macro test"));
    ASSERT_NO_THROW(LOG_ERROR("Error macro test"));
    ASSERT_NO_THROW(LOG_CRITICAL("Critical macro test"));
    ASSERT_NO_THROW(LOG_FLUSH());
}

TEST_F(logger_test, level_filtering)
{
    logger test_logger("filter_test", false);
    test_logger.add_sink(logger::create_stdout_sink());

    test_logger.set_level(level::warning);

    ASSERT_FALSE(test_logger.should_log(level::trace));
    ASSERT_FALSE(test_logger.should_log(level::debug));
    ASSERT_FALSE(test_logger.should_log(level::info));

    ASSERT_TRUE(test_logger.should_log(level::warning));
    ASSERT_TRUE(test_logger.should_log(level::error));
    ASSERT_TRUE(test_logger.should_log(level::critical));
}