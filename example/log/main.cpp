#include <iostream>

#include <libcpp/log/log.hpp>

int main(int argc, char* argv[])
{
    // do logging by default logger
    LOG_TRACE("LOG_TRACE {}", "hello");
    LOG_DEBUG("LOG_DEBUG d{0:d} 0x{0:x} o{0:o} b{0:b}", 10);
    LOG_INFO("LOG_INFO {0:03.2f}", 123.45);
    LOG_WARN("LOG_WARN {1} {0}", "hello", "world");
    LOG_ERROR("LOG_ERROR");
    LOG_CRITICAL("LOG_CRITICAL");
    LOG_FLUSH();

    // do sync logging by user defined rotate file logger
    auto lg1 = std::make_shared<libcpp::logger>("lg1");
    lg1->add_sink(libcpp::logger::create_stdout_sink());
    lg1->add_sink(libcpp::logger::create_rotate_file_sink("logs/libcpp.log", 
                                                          2 * 1024 * 1024, 
                                                          2,
                                                          false));
    lg1->set_pattern("%Y-%m-%d %H:%M:%S.%e [%-8l] %v");
    lg1->trace("lg1 LOG_TRACE {}", "hello");
    lg1->debug("lg1 LOG_DEBUG d{0:d} 0x{0:x} o{0:o} b{0:b}", 10);
    lg1->info("lg1 LOG_INFO {0:03.2f}", 123.45);
    lg1->warn("lg1 LOG_WARN {1} {0}", "hello", "world");
    lg1->error("lg1 LOG_ERROR");
    lg1->critical("lg1 LOG_CRITICAL");
    lg1->flush();

    // do sync logging by user defined daily logger
    auto lg2 = std::make_shared<libcpp::logger>("lg2", true);
    lg2->add_sink(libcpp::logger::create_stdout_sink());
    lg2->clear_sink();
    lg2->add_sink(libcpp::logger::create_daily_file_sink("logs/libcpp.log", 
                                                         0, 
                                                         1));
    while (true)
    {
        lg2->trace("lg2 LOG_TRACE {}", "hello");
        lg2->debug("lg2 LOG_DEBUG d{0:d} 0x{0:x} o{0:o} b{0:b}", 10);
        lg2->info("lg2 LOG_INFO {0:03.2f}", 123.45);
        lg2->warn("lg2 LOG_WARN {1} {0}", "hello", "world");
        lg2->error("lg2 LOG_ERROR");
        lg2->critical("lg2 LOG_CRITICAL");
        lg2->flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cin.get();
    return 0;
}