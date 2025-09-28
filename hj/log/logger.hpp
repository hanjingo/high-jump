#ifndef LOGGER_HPP
#define LOGGER_HPP

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <atomic>
#include <mutex>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

#if defined(QT_VERSION) && defined(QT_CORE_LIB)
#ifndef HJ_QT_ENVIRONMENT
#define HJ_QT_ENVIRONMENT 1
#endif

#include <QString>
#include <QDebug>
#include <QLoggingCategory>
#else
#ifndef HJ_QT_ENVIRONMENT
#define HJ_QT_ENVIRONMENT 0
#endif
#endif

#ifndef LOG_QUEUE_SIZE
#define LOG_QUEUE_SIZE 102400
#endif

#ifndef LOG_THREAD_NUM
#define LOG_THREAD_NUM 1
#endif

namespace hj
{

class logger;

enum class log_lvl : int
{
    trace = 0,
    debug,
    info,
    warn,
    err,
    critical,
};

static std::mutex log_mu;

// for qt environment
#if HJ_QT_ENVIRONMENT
static void default_qt_msg_handler(QtMsgType                 typ,
                                   const QMessageLogContext &context,
                                   const QString            &str)
{
    switch(typ)
    {
        case QtDebugMsg: {
            hj::logger::instance()->debug(str.toUtf8().constData());
            break;
        }
        case QtWarningMsg: {
            hj::logger::instance()->warn(str.toUtf8().constData());
            break;
        }
        case QtCriticalMsg: {
            hj::logger::instance()->critical(str.toUtf8().constData());
            break;
        }
        case QtInfoMsg: {
            hj::logger::instance()->info(str.toUtf8().constData());
            break;
        }
        default:
            break;
    };
};

static void install_qt_msg_handler(void (*fn)(QtMsgType,
                                              const QMessageLogContext &,
                                              const QString &))
{
    qInstallMessageHandler(fn);
}

static void uninstall_qt_msg_handler()
{
    qInstallMessageHandler(nullptr);
}
#endif

class logger
{
  public:
    using sink_ptr_t        = spdlog::sink_ptr;
    using base_logger_ptr_t = std::shared_ptr<spdlog::logger>;

  public:
    logger()                          = delete;
    logger(const logger &)            = delete;
    logger &operator=(const logger &) = delete;

    logger(base_logger_ptr_t base)
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
            spdlog::init_thread_pool(queue_size, thread_num);

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

        _base->set_level(spdlog::level::level_enum(log_lvl::trace));
    }

    ~logger()
    {
        flush();
        spdlog::drop_all();
    }

    static hj::logger *instance()
    {
        static hj::logger inst{spdlog::default_logger()};
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

    inline void add_sink(sink_ptr_t &&sink)
    {
        std::lock_guard<std::mutex> lock(log_mu);
        _base->sinks().push_back(std::move(sink));
    }

    inline void remove_sink(const sink_ptr_t sink)
    {
        std::lock_guard<std::mutex> lock(log_mu);
        _base->sinks().erase(
            std::remove(_base->sinks().begin(), _base->sinks().end(), sink),
            _base->sinks().end());
    }

    inline void clear_sink()
    {
        std::lock_guard<std::mutex> lock(log_mu);
        _base->sinks().clear();
    }

    inline void set_level(const hj::log_lvl lvl)
    {
        _base->set_level(
            static_cast<spdlog::level::level_enum>(static_cast<int>(lvl)));
    }

    inline log_lvl get_level() { return static_cast<log_lvl>(_base->level()); }

    // see also: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    inline void set_pattern(const char *patterm)
    {
        _base->set_pattern(patterm);
    }

    inline void flush() { _base->flush(); }

    inline void flush_on(const log_lvl lvl)
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
    base_logger_ptr_t _base;
};

}

#define LOG_TRACE(...) hj::logger::instance()->trace(__VA_ARGS__)

#define LOG_DEBUG(...) hj::logger::instance()->debug(__VA_ARGS__)

#define LOG_INFO(...) hj::logger::instance()->info(__VA_ARGS__)

#define LOG_WARN(...) hj::logger::instance()->warn(__VA_ARGS__)

#define LOG_ERROR(...) hj::logger::instance()->error(__VA_ARGS__)

#define LOG_CRITICAL(...) hj::logger::instance()->critical(__VA_ARGS__)

#define LOG_FLUSH() hj::logger::instance()->flush();

#endif