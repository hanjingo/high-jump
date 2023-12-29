#include <iostream>
#include <libcpp/log/log.hpp>

int main(int argc, char* argv[])
{
    libcpp::log_config cfg;
    cfg.sink = libcpp::log_sink_console | libcpp::log_sink_file;

    auto lg = std::make_shared<libcpp::logger>(cfg);
    libcpp::logger::set_default_logger(lg);
    lg->set_level(libcpp::log_lvl_debug);

    // see also: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    lg->set_pattern("%Y-%m-%d %H:%M:%S.%e[%-8l][%@] %v");

    LOG_TRACE("LOG_TRACE {}", "hello");
    libcpp::logger::instance()->trace("trace {}", "hello");

    LOG_DEBUG("LOG_DEBUG {0:d} 0x{0:x} {0:o} {0:b}", 10);
    libcpp::logger::instance()->debug("debug");

    LOG_INFO("LOG_INFO {0:03.2f}", 123.45);
    libcpp::logger::instance()->info("info {0:03.2f}", 123.45);

    LOG_WARN("LOG_WARN {1} {0}", "hello", "world");
    libcpp::logger::instance()->warn("warn");

    LOG_ERROR("LOG_ERROR");
    libcpp::logger::instance()->error("error");

    LOG_CRITICAL("LOG_CRITICAL");
    libcpp::logger::instance()->critical("critical");

    libcpp::logger::instance()->flush();

    std::cin.get();
    return 0;
}