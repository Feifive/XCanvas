#include "AppLogging.h"

#include <QByteArray>
#include <QDir>
#include <QByteArrayView>
#include <QProcessEnvironment>
#include <QMessageLogContext>
#include <QStandardPaths>
#include <QStringList>

#include <atomic>
#include <cstdio>
#include <memory>
#include <mutex>

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace
{
constexpr auto     kLoggerName         = "xcanvas";
constexpr auto     kModuleNamePrefix   = "xcanvas.";
constexpr auto     kLogFileName        = "xcanvas.log";
constexpr auto     kCompactLogPattern  = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";
constexpr auto     kVerboseLogPattern  = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [tid %t] %v";
constexpr int      kRotationHour       = 0;
constexpr int      kRotationMinute     = 0;
constexpr uint16_t kMaxLogFiles        = 30;

QtMessageHandler g_previousQtMessageHandler = nullptr;
bool             g_loggingInitialized       = false;
std::mutex       g_loggerMutex;
std::atomic<AppLogging::LogDetailMode> g_logDetailMode{AppLogging::LogDetailMode::Compact};

spdlog::level::level_enum defaultLogLevel()
{
#ifdef QT_DEBUG
    return spdlog::level::debug;
#else
    return spdlog::level::info;
#endif
}

spdlog::level::level_enum defaultFlushLevel()
{
#ifdef QT_DEBUG
    return spdlog::level::debug;
#else
    return spdlog::level::warn;
#endif
}

spdlog::level::level_enum toSpdlogLevel(const QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:
            return spdlog::level::debug;
        case QtInfoMsg:
            return spdlog::level::info;
        case QtWarningMsg:
            return spdlog::level::warn;
        case QtCriticalMsg:
            return spdlog::level::err;
        case QtFatalMsg:
            return spdlog::level::critical;
    }

    return spdlog::level::info;
}

AppLogging::LogDetailMode parseLogDetailMode(const QByteArrayView modeText)
{
    const QByteArray normalized = modeText.toByteArray().trimmed().toLower();
    if (normalized == "verbose" || normalized == "detail" || normalized == "detailed")
    {
        return AppLogging::LogDetailMode::Verbose;
    }

    return AppLogging::LogDetailMode::Compact;
}

AppLogging::LogDetailMode defaultLogDetailMode()
{
    const QString envValue = QProcessEnvironment::systemEnvironment().value(QStringLiteral("XCANVAS_LOG_DETAIL"));
    if (!envValue.isEmpty())
    {
        return parseLogDetailMode(QByteArrayView(envValue.toUtf8()));
    }

    return AppLogging::LogDetailMode::Compact;
}

const char* sharedLogPattern()
{
    return g_logDetailMode.load(std::memory_order_relaxed) == AppLogging::LogDetailMode::Verbose ? kVerboseLogPattern
                                                                                                  : kCompactLogPattern;
}

void applyPatternToLoggerSinks(const std::shared_ptr<spdlog::logger>& logger)
{
    if (!logger)
    {
        return;
    }

    for (const auto& sink : logger->sinks())
    {
        if (sink)
        {
            sink->set_pattern(sharedLogPattern());
        }
    }
}

std::shared_ptr<spdlog::logger> createConsoleFallbackLogger()
{
    auto fallbackSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    fallbackSink->set_pattern(sharedLogPattern());

    auto fallbackLogger = std::make_shared<spdlog::logger>(kLoggerName, std::move(fallbackSink));
    fallbackLogger->set_level(defaultLogLevel());
    fallbackLogger->flush_on(defaultFlushLevel());
    return fallbackLogger;
}

std::shared_ptr<spdlog::logger> ensureRootLoggerUnlocked()
{
    if (auto logger = spdlog::default_logger())
    {
        return logger;
    }

    auto fallbackLogger = createConsoleFallbackLogger();
    spdlog::set_default_logger(fallbackLogger);
    spdlog::set_level(defaultLogLevel());
    spdlog::flush_on(defaultFlushLevel());
    return fallbackLogger;
}

std::shared_ptr<spdlog::logger> createModuleLoggerUnlocked(const std::string& loggerName)
{
    if (auto existing = spdlog::get(loggerName))
    {
        return existing;
    }

    const auto root = ensureRootLoggerUnlocked();
    auto moduleLogger = std::make_shared<spdlog::logger>(loggerName, root->sinks().begin(), root->sinks().end());
    moduleLogger->set_level(root->level());
    moduleLogger->flush_on(root->flush_level());
    spdlog::register_logger(moduleLogger);
    return moduleLogger;
}

QString buildQtMessage(const QMessageLogContext& context, const QString& message)
{
    QString formattedMessage = message;
    if (formattedMessage.isNull())
    {
        formattedMessage = QStringLiteral("<null>");
    }

    if (AppLogging::logDetailMode() == AppLogging::LogDetailMode::Compact)
    {
        if (context.category != nullptr && context.category[0] != '\0')
        {
            return QStringLiteral("[%1] %2").arg(QString::fromUtf8(context.category), formattedMessage);
        }
        return formattedMessage;
    }

    QStringList parts;
    if (context.category != nullptr && context.category[0] != '\0')
    {
        parts << QString::fromUtf8(context.category);
    }

    if (context.file != nullptr && context.file[0] != '\0')
    {
        QString location = QString::fromUtf8(context.file);
        if (context.line > 0)
        {
            location += QStringLiteral(":%1").arg(context.line);
        }
        parts << location;
    }

    if (context.function != nullptr && context.function[0] != '\0')
    {
        parts << QString::fromUtf8(context.function);
    }

    if (parts.isEmpty())
    {
        return formattedMessage;
    }

    return QStringLiteral("[%1] %2").arg(parts.join(QStringLiteral(" | ")), formattedMessage);
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    const QString formattedMessage = buildQtMessage(context, message);
    const auto    logger           = spdlog::default_logger();
    if (logger != nullptr)
    {
        logger->log(toSpdlogLevel(type), formattedMessage.toStdString());
        if (type == QtFatalMsg)
        {
            logger->flush();
        }
    }
    else
    {
        const QByteArray localMessage = formattedMessage.toLocal8Bit();
        std::fputs(localMessage.constData(), stderr);
        std::fputc('\n', stderr);
        std::fflush(stderr);
    }

    if (g_previousQtMessageHandler != nullptr)
    {
        g_previousQtMessageHandler(type, context, message);
    }

    if (type == QtFatalMsg)
    {
        abort();
    }
}
} // namespace

QString AppLogging::logDirectoryPath()
{
    const QString appConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir          dir(appConfigPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    return dir.filePath(QStringLiteral("Log"));
}

QString AppLogging::normalizeModuleName(const QString& moduleName)
{
    const QString trimmed = moduleName.trimmed();
    if (trimmed.isEmpty())
    {
        return QString::fromLatin1(kLoggerName);
    }

    if (trimmed == QString::fromLatin1(kLoggerName) || trimmed.startsWith(QString::fromLatin1(kModuleNamePrefix)))
    {
        return trimmed;
    }

    return QString::fromLatin1(kModuleNamePrefix) + trimmed;
}

std::shared_ptr<spdlog::logger> AppLogging::rootLogger()
{
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    return ensureRootLoggerUnlocked();
}

std::shared_ptr<spdlog::logger> AppLogging::logger(const QString& moduleName)
{
    const QString normalizedName = normalizeModuleName(moduleName);
    const auto    loggerName     = normalizedName.toStdString();

    std::lock_guard<std::mutex> lock(g_loggerMutex);
    if (loggerName == kLoggerName)
    {
        return ensureRootLoggerUnlocked();
    }

    return createModuleLoggerUnlocked(loggerName);
}

void setLogLevelUnlocked(const spdlog::level::level_enum level)
{
    ensureRootLoggerUnlocked();
    spdlog::set_level(level);
    spdlog::apply_all([level](const std::shared_ptr<spdlog::logger>& logger) {
        if (logger)
        {
            logger->set_level(level);
        }
    });
}

void AppLogging::setLogLevel(const spdlog::level::level_enum level)
{
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    setLogLevelUnlocked(level);
}

void AppLogging::setLogDetailMode(const LogDetailMode mode)
{
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    g_logDetailMode.store(mode, std::memory_order_relaxed);
    spdlog::apply_all([](const std::shared_ptr<spdlog::logger>& logger) {
        applyPatternToLoggerSinks(logger);
    });
}

AppLogging::LogDetailMode AppLogging::logDetailMode()
{
    return g_logDetailMode.load(std::memory_order_relaxed);
}

bool AppLogging::isVerboseLogDetail()
{
    return logDetailMode() == LogDetailMode::Verbose;
}

void AppLogging::initialize()
{
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    if (g_loggingInitialized)
    {
        return;
    }

    try
    {
        g_logDetailMode.store(defaultLogDetailMode(), std::memory_order_relaxed);

        const QString logDirPath = logDirectoryPath();
        QDir          logDir(logDirPath);
        if (!logDir.exists())
        {
            logDir.mkpath(".");
        }

        auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            logDir.filePath(QLatin1String(kLogFileName)).toStdString(),
            kRotationHour,
            kRotationMinute,
            false,
            kMaxLogFiles);
        fileSink->set_pattern(sharedLogPattern());

        spdlog::sinks_init_list sinks{fileSink};
#ifdef QT_DEBUG
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern(sharedLogPattern());
        sinks = {fileSink, consoleSink};
#endif
        auto logger = std::make_shared<spdlog::logger>(kLoggerName, sinks);
        logger->set_level(defaultLogLevel());
        logger->flush_on(defaultFlushLevel());

        spdlog::set_default_logger(logger);
        setLogLevelUnlocked(defaultLogLevel());
        spdlog::flush_on(defaultFlushLevel());

        g_previousQtMessageHandler = qInstallMessageHandler(qtMessageHandler);
        g_loggingInitialized       = true;

        logger->info("Application logging initialized. log_dir={}", logDirPath.toStdString());
    }
    catch (const std::exception& ex)
    {
        try
        {
            auto fallbackLogger = createConsoleFallbackLogger();

            spdlog::set_default_logger(fallbackLogger);
            setLogLevelUnlocked(defaultLogLevel());
            spdlog::flush_on(defaultFlushLevel());

            g_previousQtMessageHandler = qInstallMessageHandler(qtMessageHandler);
            g_loggingInitialized       = true;

            fallbackLogger->error("Failed to initialize file logging. Falling back to stderr logger: {}", ex.what());
        }
        catch (...)
        {
            std::fprintf(stderr, "Failed to initialize application logging: %s\n", ex.what());
            std::fflush(stderr);
        }
    }
    catch (...)
    {
        std::fputs("Failed to initialize application logging: unknown exception\n", stderr);
        std::fflush(stderr);
    }
}

void AppLogging::shutdown()
{
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    if (!g_loggingInitialized)
    {
        return;
    }

    if (auto logger = spdlog::default_logger())
    {
        logger->info("Application logging shutting down.");
        logger->flush();
    }

    qInstallMessageHandler(g_previousQtMessageHandler);
    g_previousQtMessageHandler = nullptr;
    g_loggingInitialized       = false;
    spdlog::shutdown();
}
