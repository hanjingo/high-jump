/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

#if defined(QT_VERSION) && defined(QT_CORE_LIB)
#ifndef LOG_QT_SUPPORT
#define LOG_QT_SUPPORT 1
#endif
#include <QString>
#include <QDebug>
#include <QLoggingCategory>
#else
#ifndef LOG_QT_SUPPORT
#define LOG_QT_SUPPORT 0
#endif
#endif

#ifndef LOG_QUEUE_SIZE
#define LOG_QUEUE_SIZE 1024
#endif

#ifndef LOG_THREAD_NUM
#define LOG_THREAD_NUM 1
#endif

namespace hj
{
namespace log
{

class logger;

enum class level : int
{
    trace = 0,
    debug,
    info,
    warning,
    error,
    critical,
    off
};

enum class overflow_policy
{
    block,
    overrun_oldest,
    discard_new
};

struct logger_options
{
    std::string     name       = "default";
    bool            async      = false;
    overflow_policy policy     = overflow_policy::block;
    std::size_t     queue_size = LOG_QUEUE_SIZE;
    std::size_t     thread_num = LOG_THREAD_NUM;
};

namespace detail
{
template <typename Mutex>
class proxy_sink : public spdlog::sinks::base_sink<Mutex>
{
  public:
    using sink_ptr_t  = spdlog::sink_ptr;
    using sink_list_t = std::vector<sink_ptr_t>;

    void add_sink(sink_ptr_t &&sink)
    {
        if(!sink)
            throw std::invalid_argument("invalid sink ptr");

        std::unique_lock lock(_mutex);
        auto             current = std::atomic_load(&_sinks);
        auto             next    = std::make_shared<sink_list_t>(*current);
        next->push_back(std::move(sink));
        std::atomic_store(&_sinks, next);
    }

    void remove_sink(const sink_ptr_t &sink)
    {
        std::unique_lock lock(_mutex);
        auto             current = std::atomic_load(&_sinks);
        auto             next    = std::make_shared<sink_list_t>(*current);
        next->erase(std::remove(next->begin(), next->end(), sink), next->end());
        std::atomic_store(&_sinks, next);
    }

    void clear_sink()
    {
        std::unique_lock lock(_mutex);
        auto             next = std::make_shared<sink_list_t>();
        std::atomic_store(&_sinks, next);
    }

    std::size_t sink_count() const noexcept
    {
        auto current = std::atomic_load(&_sinks);
        return current->size();
    }

  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        auto current = std::atomic_load(&_sinks);
        for(const auto &sink : *current)
        {
            if(sink->should_log(msg.level))
            {
                sink->log(msg);
            }
        }
    }

    void flush_() override
    {
        auto current = std::atomic_load(&_sinks);
        for(const auto &sink : *current)
        {
            sink->flush();
        }
    }

  private:
    std::mutex                   _mutex;
    std::shared_ptr<sink_list_t> _sinks{std::make_shared<sink_list_t>()};
};

} // namespace detail

#if LOG_QT_SUPPORT
class QtMessageHandler
{
  public:
    using Handler = std::function<void(
        QtMsgType, const QMessageLogContext &, const QString &)>;

    static void install(const Handler &handler = {},
                        const char    *name    = "default")
    {
        if(_is_installed.load())
            return;

        std::lock_guard lock(_mu);
        _name = name;
        if(handler)
        {
            _custom_handler = handler;
            qInstallMessageHandler(&QtMessageHandler::dispatch);
        } else
        {
            _custom_handler = nullptr;
            qInstallMessageHandler(&QtMessageHandler::default_handler);
        }

        _is_installed.store(true);
    }

    static void uninstall() noexcept
    {
        try
        {
            std::lock_guard lock(_mu);
            qInstallMessageHandler(nullptr);
            _custom_handler = nullptr;
            _is_installed.store(false);
        }
        catch(...)
        {
        }
    }

    static bool isInstalled() noexcept { return _is_installed.load(); }

  private:
    static void dispatch(QtMsgType                 type,
                         const QMessageLogContext &context,
                         const QString            &message)
    {
        Handler handler_copy;
        {
            std::lock_guard lock(_mu);
            handler_copy = _custom_handler;
        }

        if(handler_copy)
            handler_copy(type, context, message);
        else
            default_handler(type, context, message);
    }

    static void default_handler(QtMsgType                 type,
                                const QMessageLogContext &context,
                                const QString            &message)
    {
        (void) context;
        try
        {
            auto       *inst    = hj::log::logger::instance();
            const char *msg_str = message.toUtf8().constData();
            switch(type)
            {
                case QtDebugMsg:
                    inst->debug("{}", msg_str);
                    break;
                case QtWarningMsg:
                    inst->warn("{}", msg_str);
                    break;
                case QtCriticalMsg:
                    inst->critical("{}", msg_str);
                    break;
                case QtInfoMsg:
                    inst->info("{}", msg_str);
                    break;
                case QtFatalMsg:
                    inst->critical("{}", msg_str);
                    std::abort();
                    break;
                default:
                    break;
            }
        }
        catch(const std::exception &e)
        {
            std::fprintf(
                stderr,
                "[hj::log Error] QtMessageHandler exception caught: %s\n",
                e.what());
        }
        catch(...)
        {
            std::fprintf(
                stderr,
                "[hj::log Error] QtMessageHandler unknown exception caught.\n");
        }
    }

    inline static std::atomic<bool> _is_installed{false};
    inline static std::string       _name{};
    inline static Handler           _custom_handler{};
    inline static std::mutex        _mu{};
};

#endif // LOG_QT_SUPPORT

class logger
{
  public:
    using sink_ptr_t        = spdlog::sink_ptr;
    using base_logger_ptr_t = std::shared_ptr<spdlog::logger>;
    using proxy_sink_ptr_t  = std::shared_ptr<detail::proxy_sink<std::mutex>>;
    using thread_pool_ptr_t = std::shared_ptr<spdlog::details::thread_pool>;

  public:
    logger()                          = delete;
    logger(const logger &)            = delete;
    logger &operator=(const logger &) = delete;
    logger(std::nullptr_t)            = delete;
    logger(logger &&)                 = delete;
    logger &operator=(logger &&)      = delete;

    explicit logger(base_logger_ptr_t base)
        : _base{base}
    {
        if(!_base)
            throw std::invalid_argument("invalid base logger ptr");
    }

    logger(const logger_options &opts = {})
    {
        _proxy_sink = std::make_shared<detail::proxy_sink<std::mutex>>();
        _proxy_sink->add_sink(
            std::make_shared<spdlog::sinks::stdout_sink_mt>());

        std::vector<sink_ptr_t> sinks{_proxy_sink};

        if(opts.async)
        {
            if(opts.queue_size == 0)
                throw std::invalid_argument("invalid queue size");

            if(opts.thread_num == 0)
                throw std::invalid_argument("invalid thread num");

            _thread_pool =
                std::make_shared<spdlog::details::thread_pool>(opts.queue_size,
                                                               opts.thread_num);
            _base = std::make_shared<spdlog::async_logger>(
                opts.name,
                sinks.begin(),
                sinks.end(),
                _thread_pool,
                static_cast<spdlog::async_overflow_policy>(opts.policy));
        } else
        {
            _base = std::make_shared<spdlog::logger>(opts.name,
                                                     sinks.begin(),
                                                     sinks.end());
        }

        _base->set_level(spdlog::level::level_enum(level::trace));
    }

    ~logger() noexcept
    {
        try
        {
            flush();
        }
        catch(...)
        {
        }
    }

    static hj::log::logger *instance()
    {
        static std::once_flag                   init_flag;
        static std::unique_ptr<hj::log::logger> inst;
        std::call_once(init_flag, []() {
            logger_options opt;
            inst = std::make_unique<hj::log::logger>(opt);
        });
        return inst.get();
    }

    static inline sink_ptr_t create_stdout_sink()
    {
        return std::make_shared<spdlog::sinks::stdout_sink_mt>();
    }

    static inline sink_ptr_t
    create_rotate_file_sink(const std::string &base_filename,
                            const std::size_t  max_size,
                            const std::size_t  max_files,
                            const bool         rotate_on_open = false)
    {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            base_filename,
            max_size,
            max_files,
            rotate_on_open);
    }

    static inline sink_ptr_t
    create_daily_file_sink(const std::string &base_filename,
                           const int          rotation_hour,
                           const int          rotation_minute,
                           const bool         truncate  = false,
                           const uint16_t     max_files = 0)
    {
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            base_filename,
            rotation_hour,
            rotation_minute,
            truncate,
            max_files);
    }

    inline const std::string &name() const noexcept { return _base->name(); }

    inline bool should_log(const level lvl) const noexcept
    {
        return _base
               && _base->should_log(static_cast<spdlog::level::level_enum>(
                   static_cast<int>(lvl)));
    }

    inline void add_sink(sink_ptr_t &&sink)
    {
        std::unique_lock lock(_mu);
        if(_proxy_sink)
            _proxy_sink->add_sink(std::forward<sink_ptr_t>(sink));
    }

    inline void remove_sink(const sink_ptr_t &sink)
    {
        std::unique_lock lock(_mu);
        if(_proxy_sink)
            _proxy_sink->remove_sink(sink);
    }

    inline void clear_sink()
    {
        std::unique_lock lock(_mu);
        if(_proxy_sink)
            _proxy_sink->clear_sink();
    }

    inline std::size_t sink_count() const noexcept
    {
        return _proxy_sink ? _proxy_sink->sink_count() : 0;
    }

    inline void set_level(const hj::log::level lvl)
    {
        _base->set_level(
            static_cast<spdlog::level::level_enum>(static_cast<int>(lvl)));
    }

    inline level get_level() const noexcept
    {
        return static_cast<level>(_base->level());
    }

    // see also: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    inline void set_pattern(const char *patterm)
    {
        _base->set_pattern(patterm);
    }

    inline void flush() { _base->flush(); }

    inline void flush_on(const level lvl)
    {
        _base->flush_on(
            static_cast<spdlog::level::level_enum>(static_cast<int>(lvl)));
    }

    inline spdlog::logger *raw() noexcept { return _base.get(); }

    inline const spdlog::logger *raw() const noexcept { return _base.get(); }

  public:
    template <typename... Args>
    inline void trace(const char *fmt, Args &&...args)
    {
        _base->trace(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void debug(const char *fmt, Args &&...args)
    {
        _base->debug(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void info(const char *fmt, Args &&...args)
    {
        _base->info(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void warn(const char *fmt, Args &&...args)
    {
        _base->warn(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void error(const char *fmt, Args &&...args)
    {
        _base->error(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(const char *fmt, Args &&...args)
    {
        _base->critical(::fmt::runtime(fmt), std::forward<Args>(args)...);
    }

  private:
    mutable std::shared_mutex _mu;
    proxy_sink_ptr_t          _proxy_sink;
    thread_pool_ptr_t         _thread_pool;
    base_logger_ptr_t         _base;
};

} // namespace log
} // namespace hj

#define LOG_TRACE(...)                                                         \
    SPDLOG_LOGGER_TRACE(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    SPDLOG_LOGGER_DEBUG(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_INFO(...)                                                          \
    SPDLOG_LOGGER_INFO(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_WARN(...)                                                          \
    SPDLOG_LOGGER_WARN(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    SPDLOG_LOGGER_ERROR(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_CRITICAL(...)                                                      \
    SPDLOG_LOGGER_CRITICAL(hj::log::logger::instance()->raw(), __VA_ARGS__)
#define LOG_FLUSH() hj::log::logger::instance()->flush()

#endif