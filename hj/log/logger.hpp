/*
 *  This file is part of hj.
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

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
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
            qInstallMessageHandler(handler);
        else
            qInstallMessageHandler(default_handler);

        _is_installed.store(true);
    }

    static void uninstall() noexcept
    {
        try
        {
            std::lock_guard lock(_mu);
            qInstallMessageHandler(nullptr);
            _is_installed.store(false);
        }
        catch(...)
        {
        }
    }

    static bool isInstalled() noexcept { return _is_installed.load(); }

  private:
    static void defaultHandler(QtMsgType                 type,
                               const QMessageLogContext &context,
                               const QString            &message)
    {
        try
        {
            switch(type)
            {
                case QtDebugMsg: {
                    hj::logger::instance()->debug(message.toUtf8().constData());
                    break;
                }
                case QtWarningMsg: {
                    hj::logger::instance()->warn(message.toUtf8().constData());
                    break;
                }
                case QtCriticalMsg: {
                    hj::logger::instance()->critical(
                        message.toUtf8().constData());
                    break;
                }
                case QtInfoMsg: {
                    hj::logger::instance()->info(message.toUtf8().constData());
                    break;
                }
                default:
                    break;
            };
        }
        catch(...)
        {
        }
    }

    static std::atomic<bool> _is_installed;
    static std::string       _name;
    static std::mutex        _mu;
};

#endif // LOG_QT_SUPPORT

class logger
{
  public:
    using sink_ptr_t        = spdlog::sink_ptr;
    using base_logger_ptr_t = std::shared_ptr<spdlog::logger>;

  public:
    logger()                              = delete;
    logger(const logger &)                = delete;
    logger &operator=(const logger &)     = delete;
    logger(logger &&) noexcept            = default;
    logger &operator=(logger &&) noexcept = default;

    explicit logger(base_logger_ptr_t base)
        : _base{base}
    {
    }

    logger(const std::string &name,
           const bool         async      = false,
           const uint64_t     queue_size = LOG_QUEUE_SIZE,
           const uint64_t     thread_num = LOG_THREAD_NUM)
    {
        std::vector<sink_ptr_t> sinks{};

        if(async)
        {
            static std::once_flag init_flag;
            std::call_once(init_flag, [&queue_size, &thread_num]() {
                spdlog::init_thread_pool(queue_size, thread_num);
            });

            _base = std::make_shared<spdlog::async_logger>(
                name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::overrun_oldest);
        } else
        {
            _base = std::make_shared<spdlog::logger>(name,
                                                     sinks.begin(),
                                                     sinks.end());
        }

        _base->set_level(spdlog::level::level_enum(level::trace));
    }

    ~logger() noexcept { flush(); }

    static hj::log::logger *instance()
    {
        static hj::log::logger inst{spdlog::default_logger()};
        return &inst;
    }

    static sink_ptr_t create_stdout_sink()
    {
        return std::make_shared<spdlog::sinks::stdout_sink_mt>();
    }

    static sink_ptr_t create_rotate_file_sink(const std::string &base_filename,
                                              const std::size_t  max_size,
                                              const std::size_t  max_files,
                                              const bool rotate_on_open = false)
    {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            base_filename,
            max_size,
            max_files,
            rotate_on_open);
    }

    static sink_ptr_t create_daily_file_sink(const std::string &base_filename,
                                             const int          rotation_hour,
                                             const int          rotation_minute,
                                             const bool     truncate  = false,
                                             const uint16_t max_files = 0)
    {
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            base_filename,
            rotation_hour,
            rotation_minute,
            truncate,
            max_files);
    }

    inline const std::string &name() { return _base->name(); }

    inline const bool should_log(const level lvl)
    {
        return _base
               && _base->should_log(static_cast<spdlog::level::level_enum>(
                   static_cast<int>(lvl)));
    }

    inline void add_sink(sink_ptr_t &&sink)
    {
        std::unique_lock lock(_mu);
        _base->sinks().push_back(std::move(sink));
    }

    inline void remove_sink(const sink_ptr_t sink)
    {
        std::unique_lock lock(_mu);
        _base->sinks().erase(
            std::remove(_base->sinks().begin(), _base->sinks().end(), sink),
            _base->sinks().end());
    }

    inline void clear_sink()
    {
        std::unique_lock lock(_mu);
        _base->sinks().clear();
    }

    inline std::size_t sink_count()
    {
        std::unique_lock lock(_mu);
        return _base->sinks().size();
    }

    inline void set_level(const hj::log::level lvl)
    {
        _base->set_level(
            static_cast<spdlog::level::level_enum>(static_cast<int>(lvl)));
    }

    inline level get_level() { return static_cast<level>(_base->level()); }

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

  public:
    template <typename... Args>
    inline void trace(const char *fmt, Args &&...args)
    {
        _base->trace(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void debug(const char *fmt, Args &&...args)
    {
        _base->debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void info(const char *fmt, Args &&...args)
    {
        _base->info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void warn(const char *fmt, Args &&...args)
    {
        _base->warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void error(const char *fmt, Args &&...args)
    {
        _base->error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(const char *fmt, Args &&...args)
    {
        _base->critical(fmt, std::forward<Args>(args)...);
    }

  private:
    mutable std::shared_mutex _mu;
    base_logger_ptr_t         _base;
};

} // namespace log
} // namespace hj

#define LOG_TRACE(...) hj::log::logger::instance()->trace(__VA_ARGS__)

#define LOG_DEBUG(...) hj::log::logger::instance()->debug(__VA_ARGS__)

#define LOG_INFO(...) hj::log::logger::instance()->info(__VA_ARGS__)

#define LOG_WARN(...) hj::log::logger::instance()->warn(__VA_ARGS__)

#define LOG_ERROR(...) hj::log::logger::instance()->error(__VA_ARGS__)

#define LOG_CRITICAL(...) hj::log::logger::instance()->critical(__VA_ARGS__)

#define LOG_FLUSH() hj::log::logger::instance()->flush();

#endif