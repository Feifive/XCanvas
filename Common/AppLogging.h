#ifndef APPLOGGING_H
#define APPLOGGING_H

#include <QString>

#include <memory>

namespace spdlog
{
class logger;
namespace level
{
enum level_enum : int;
}
}

namespace AppLogging
{
    enum class LogDetailMode
    {
        Compact,
        Verbose
    };

    void initialize();
    void shutdown();
    QString logDirectoryPath();
    QString normalizeModuleName(const QString& moduleName);
    std::shared_ptr<spdlog::logger> rootLogger();
    std::shared_ptr<spdlog::logger> logger(const QString& moduleName);
    void setLogLevel(spdlog::level::level_enum level);
    void setLogDetailMode(LogDetailMode mode);
    LogDetailMode logDetailMode();
    bool isVerboseLogDetail();
}

#endif// APPLOGGING_H
