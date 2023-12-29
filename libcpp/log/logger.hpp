#ifndef LOGGER_HPP
#define LOGGER_HPP

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <atomic>
#include <string>

#include <spdlog/spdlog.h>

namespace libcpp
{

enum log_sink : uint32_t {
    log_sink_console = 1,
    log_sink_file    = 1 << 1,
};

enum log_lvl {
    log_lvl_trace = 0,
    log_lvl_debug,
    log_lvl_info,
    log_lvl_warn,
    log_lvl_err,
    log_lvl_critial,
};

enum log_mode {
    log_mode_sync,
    log_mode_async
};

struct log_config {
    std::string id       = "";
    log_mode mode        = log_mode_sync;
    int sink             = log_sink_console;

    // file
    std::string file_path  = "logs/libcpp.log";
    uint64_t max_file_size = 2 * 1024 * 1024; // 2MB
    uint64_t max_file_num  = 200000;          // 2MB * 200000 ~= 400GB

    // async
    uint64_t queue_size = 102400;
    uint64_t thread_num = 1;
};

class logger;
static std::shared_ptr<libcpp::logger> log_inst;

class logger
{
public:
    logger() = delete;
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    logger(const libcpp::log_config& cfg)
    {
        if (cfg.mode == libcpp::log_mode_async) {
            spdlog::set_async_mode(cfg.queue_size);
        }

        std::vector<spdlog::sink_ptr> sinks{};

        if (cfg.sink & libcpp::log_sink_console) {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        }

        if (cfg.sink & libcpp::log_sink_file) {
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                cfg.file_path, cfg.max_file_size, cfg.max_file_num));
        }

        if (cfg.mode == libcpp::log_mode_sync) {
            base_ = std::make_shared<spdlog::logger>(cfg.id, sinks.begin(), sinks.end());
        }
    }

    ~logger()
    {
        flush();
        spdlog::drop_all();
    }

    static std::shared_ptr<libcpp::logger> instance()
    {
        return log_inst;
    }

    static void set_default_logger(std::shared_ptr<libcpp::logger> inst)
    {
        log_inst = inst;
    }

    inline void set_level(libcpp::log_lvl lvl)
    {
        base_->set_level(spdlog::level::level_enum(lvl));
    }

    inline void set_pattern(const char* patterm)
    {
        base_->set_pattern(patterm);
    }

    inline void flush()
    {
        base_->flush();
    }

public:
    template<typename... Args>
    inline void trace(const char* fmt, Args&& ... args)
    {
        base_->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(const char* fmt, Args&& ... args)
    {
        base_->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void info(const char* fmt, Args&& ... args)
    {
        base_->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(const char* fmt, Args&& ... args)
    {
        base_->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(const char* fmt, Args&& ... args)
    {
        base_->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const char* fmt, Args&& ... args)
    {
        base_->critical(fmt, std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<spdlog::logger> base_;
};

}

#define LOG_TRACE(...) \
    libcpp::logger::instance()->trace(__VA_ARGS__)

#define LOG_DEBUG(...) \
    libcpp::logger::instance()->debug(__VA_ARGS__)

#define LOG_INFO(...) \
    libcpp::logger::instance()->info(__VA_ARGS__)

#define LOG_WARN(...) \
    libcpp::logger::instance()->warn(__VA_ARGS__)

#define LOG_ERROR(...) \
    libcpp::logger::instance()->error(__VA_ARGS__)

#define LOG_CRITICAL(...) \
    libcpp::logger::instance()->critical(__VA_ARGS__)

#endif