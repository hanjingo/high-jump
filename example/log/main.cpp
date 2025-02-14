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

    // do sync logging by user defined logger
    auto lg1 = std::make_shared<libcpp::logger>("lg1");
    lg1->add_sink(libcpp::logger::create_sink());
    lg1->add_sink(libcpp::logger::create_sink("logs/libcpp.log", 2 * 1024 * 1024, 200000));
    lg1->set_pattern("%Y-%m-%d %H:%M:%S.%e [%-8l] %v");
    libcpp::logger::set_default(std::move(lg1));
    LOG_TRACE("LOG_TRACE {}", "hello");
    LOG_DEBUG("LOG_DEBUG d{0:d} 0x{0:x} o{0:o} b{0:b}", 10);
    LOG_INFO("LOG_INFO {0:03.2f}", 123.45);
    LOG_WARN("LOG_WARN {1} {0}", "hello", "world");
    LOG_ERROR("LOG_ERROR");
    LOG_CRITICAL("LOG_CRITICAL");
    LOG_FLUSH();

    // do async logging by user defined logger
    auto lg2 = std::make_shared<libcpp::logger>("lg2", true);
    lg2->add_sink(libcpp::logger::create_sink());
    lg2->add_sink(libcpp::logger::create_sink("logs/libcpp.log", 2 * 1024 * 1024, 200000));
    libcpp::logger::set_default(std::move(lg2));
    LOG_TRACE("LOG_TRACE {}", "hello");
    LOG_DEBUG("LOG_DEBUG d{0:d} 0x{0:x} o{0:o} b{0:b}", 10);
    LOG_INFO("LOG_INFO {0:03.2f}", 123.45);
    LOG_WARN("LOG_WARN {1} {0}", "hello", "world");
    LOG_ERROR("LOG_ERROR");
    LOG_CRITICAL("LOG_CRITICAL");
    LOG_FLUSH();

    std::cin.get();
    return 0;
}