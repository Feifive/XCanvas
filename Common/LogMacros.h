#ifndef XCANVAS_LOGMACROS_H
#define XCANVAS_LOGMACROS_H

#include "AppLogging.h"
#include "QtFmtSupport.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QString>
#include <QThread>
#include <spdlog/logger.h>

#include <memory>
#include <string>
#include <utility>

namespace xc_log
{

inline QByteArray toUtf8(const QString& s)
{
    return s.toUtf8();
}

inline QByteArray toUtf8(const QByteArray& s)
{
    return s;
}

inline QByteArray toUtf8(const std::string& s)
{
    return QByteArray::fromStdString(s);
}

inline QByteArray toUtf8(const char* s)
{
    return s ? QByteArray(s) : QByteArray();
}

inline const char* fileNameOnly(const char* file)
{
    if (file == nullptr)
    {
        return "";
    }

    const char* slash1 = strrchr(file, '/');
    const char* slash2 = strrchr(file, '\\');
    const char* slash  = slash1 > slash2 ? slash1 : slash2;
    return slash ? slash + 1 : file;
}

inline quintptr currentThreadIdValue()
{
    return reinterpret_cast<quintptr>(QThread::currentThreadId());
}

class ScopeTimer
{
public:
    ScopeTimer(std::shared_ptr<spdlog::logger> logger, const char* file, int line, const char* function, QString name)
        : m_logger(std::move(logger))
        , m_file(fileNameOnly(file))
        , m_line(line)
        , m_function(function ? function : "")
        , m_name(std::move(name))
    {
        m_timer.start();

        if (m_logger)
        {
            const QByteArray nameUtf8 = m_name.toUtf8();
            if (AppLogging::isVerboseLogDetail())
            {
                m_logger->debug("[{}:{} | {}] >>> {}", m_file, m_line, m_function, nameUtf8.constData());
            }
            else
            {
                m_logger->debug(">>> {}", nameUtf8.constData());
            }
        }
    }

    ~ScopeTimer()
    {
        if (m_logger)
        {
            const QByteArray nameUtf8 = m_name.toUtf8();
            if (AppLogging::isVerboseLogDetail())
            {
                m_logger->debug("[{}:{} | {}] <<< {} ({} ms)", m_file, m_line, m_function, nameUtf8.constData(), m_timer.elapsed());
            }
            else
            {
                m_logger->debug("<<< {} ({} ms)", nameUtf8.constData(), m_timer.elapsed());
            }
        }
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    const char*   m_file = "";
    int           m_line = 0;
    const char*   m_function = "";
    QString       m_name;
    QElapsedTimer m_timer;
};

} // namespace xc_log

#define XC_LOG_IMPL(module, level, fmt, ...)                                                          \
    do                                                                                                \
    {                                                                                                 \
        static const auto _xc_logger = AppLogging::logger(QStringLiteral(module));                    \
        if (_xc_logger)                                                                               \
        {                                                                                             \
            if (AppLogging::isVerboseLogDetail())                                                     \
            {                                                                                         \
                _xc_logger->level("[{}:{} | {}] " fmt, ::xc_log::fileNameOnly(__FILE__), __LINE__, Q_FUNC_INFO, ##__VA_ARGS__); \
            }                                                                                         \
            else                                                                                      \
            {                                                                                         \
                _xc_logger->level(fmt, ##__VA_ARGS__);                                                \
            }                                                                                         \
        }                                                                                             \
    } while (0)

#define XC_LOG_TRACE(module, fmt, ...)    XC_LOG_IMPL(module, trace, fmt, ##__VA_ARGS__)
#define XC_LOG_DEBUG(module, fmt, ...)    XC_LOG_IMPL(module, debug, fmt, ##__VA_ARGS__)
#define XC_LOG_INFO(module, fmt, ...)     XC_LOG_IMPL(module, info, fmt, ##__VA_ARGS__)
#define XC_LOG_WARN(module, fmt, ...)     XC_LOG_IMPL(module, warn, fmt, ##__VA_ARGS__)
#define XC_LOG_ERROR(module, fmt, ...)    XC_LOG_IMPL(module, error, fmt, ##__VA_ARGS__)
#define XC_LOG_CRITICAL(module, fmt, ...) XC_LOG_IMPL(module, critical, fmt, ##__VA_ARGS__)

#define XC_SCOPE_LOG(module, name)                                                                    \
    static const auto _xc_scope_logger_##__LINE__ = AppLogging::logger(QStringLiteral(module));      \
    ::xc_log::ScopeTimer _xc_scope_timer_##__LINE__(_xc_scope_logger_##__LINE__, __FILE__, __LINE__, \
                                                    Q_FUNC_INFO, (name))

#define DXF_LOG_TRACE(fmt, ...)    XC_LOG_TRACE("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_LOG_DEBUG(fmt, ...)    XC_LOG_DEBUG("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_LOG_INFO(fmt, ...)     XC_LOG_INFO("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_LOG_WARN(fmt, ...)     XC_LOG_WARN("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_LOG_ERROR(fmt, ...)    XC_LOG_ERROR("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_LOG_CRITICAL(fmt, ...) XC_LOG_CRITICAL("import.dxf", fmt, ##__VA_ARGS__)
#define DXF_SCOPE_LOG(name)        XC_SCOPE_LOG("import.dxf", (name))

#define CANVAS_LOG_TRACE(fmt, ...)    XC_LOG_TRACE("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_LOG_DEBUG(fmt, ...)    XC_LOG_DEBUG("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_LOG_INFO(fmt, ...)     XC_LOG_INFO("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_LOG_WARN(fmt, ...)     XC_LOG_WARN("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_LOG_ERROR(fmt, ...)    XC_LOG_ERROR("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_LOG_CRITICAL(fmt, ...) XC_LOG_CRITICAL("canvas", fmt, ##__VA_ARGS__)
#define CANVAS_SCOPE_LOG(name)        XC_SCOPE_LOG("canvas", (name))

#define SETTINGS_LOG_TRACE(fmt, ...)    XC_LOG_TRACE("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_LOG_DEBUG(fmt, ...)    XC_LOG_DEBUG("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_LOG_INFO(fmt, ...)     XC_LOG_INFO("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_LOG_WARN(fmt, ...)     XC_LOG_WARN("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_LOG_ERROR(fmt, ...)    XC_LOG_ERROR("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_LOG_CRITICAL(fmt, ...) XC_LOG_CRITICAL("settings", fmt, ##__VA_ARGS__)
#define SETTINGS_SCOPE_LOG(name)        XC_SCOPE_LOG("settings", (name))

#define DEVICE_LOG_TRACE(fmt, ...)    XC_LOG_TRACE("device", fmt, ##__VA_ARGS__)
#define DEVICE_LOG_DEBUG(fmt, ...)    XC_LOG_DEBUG("device", fmt, ##__VA_ARGS__)
#define DEVICE_LOG_INFO(fmt, ...)     XC_LOG_INFO("device", fmt, ##__VA_ARGS__)
#define DEVICE_LOG_WARN(fmt, ...)     XC_LOG_WARN("device", fmt, ##__VA_ARGS__)
#define DEVICE_LOG_ERROR(fmt, ...)    XC_LOG_ERROR("device", fmt, ##__VA_ARGS__)
#define DEVICE_LOG_CRITICAL(fmt, ...) XC_LOG_CRITICAL("device", fmt, ##__VA_ARGS__)
#define DEVICE_SCOPE_LOG(name)        XC_SCOPE_LOG("device", (name))

#define DOCUMENT_IO_LOG_TRACE(fmt, ...)    XC_LOG_TRACE("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_LOG_DEBUG(fmt, ...)    XC_LOG_DEBUG("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_LOG_INFO(fmt, ...)     XC_LOG_INFO("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_LOG_WARN(fmt, ...)     XC_LOG_WARN("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_LOG_ERROR(fmt, ...)    XC_LOG_ERROR("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_LOG_CRITICAL(fmt, ...) XC_LOG_CRITICAL("documentIO", fmt, ##__VA_ARGS__)
#define DOCUMENT_IO_SCOPE_LOG(name)        XC_SCOPE_LOG("documentIO", (name))

#endif //XCANVAS_LOGMACROS_H
