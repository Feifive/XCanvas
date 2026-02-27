#include "AppSettings.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

AppSettings::GridContrast AppSettings::gridContrast() const
{
    return m_gridContrast;
}

void AppSettings::setGridContrast(GridContrast contrast)
{
    if (m_gridContrast != contrast)
    {
        m_gridContrast = contrast;
        emit gridContrastChanged();
        emit settingsChanged();
    }
}

AppSettings::AppSettings(QObject* parent) : QObject(parent)
{
    load();
}

AppSettings::~AppSettings()
{
    save();
}

QString AppSettings::getConfigFilePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    qDebug() << path;
    QDir dir(path);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    return dir.filePath("config.json");
}

void AppSettings::load()
{
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "No config file found, using defaults.";
        return;
    }

    QByteArray    data = file.readAll();
    QJsonDocument doc  = QJsonDocument::fromJson(data);
    QJsonObject   json = doc.object();


    m_gridContrast   = static_cast<GridContrast>(json["showGrid"].toInt(static_cast<int>(AppSettings::GridContrast::Medium)));
    m_lastOpenedPath = json["lastOpenedPath"].toString();
}

void AppSettings::save()
{
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Couldn't open config file for writing.";
        return;
    }

    QJsonObject json;
    json["showGrid"]       = static_cast<int>(m_gridContrast);
    json["lastOpenedPath"] = m_lastOpenedPath;

    QJsonDocument doc(json);
    file.write(doc.toJson());
}

QString AppSettings::lastOpenedPath() const
{
    return m_lastOpenedPath;
}

QString AppSettings::lastOpenedPathOrDocumentsPath() const
{
    if (!m_lastOpenedPath.isEmpty())
    {
        return m_lastOpenedPath;
    }

    const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return documentsPath.isEmpty() ? QDir::homePath() : documentsPath;
}

void AppSettings::setLastOpenedPath(const QString& path)
{
    if (path.isEmpty())
    {
        if (!m_lastOpenedPath.isEmpty())
        {
            m_lastOpenedPath.clear();
            emit settingsChanged();
        }
        return;
    }

    QFileInfo fileInfo(path);
    const QString normalizedPath = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.absolutePath();
    if (normalizedPath.isEmpty() || m_lastOpenedPath == normalizedPath)
    {
        return;
    }

    m_lastOpenedPath = normalizedPath;
    emit settingsChanged();
}

QColor AppSettings::activeColor() const {
    return m_activeColor;
}

void AppSettings::setActiveColor(const QColor &color) {
    if (m_activeColor !=  color) {
        m_activeColor = color;
        emit settingsChanged();
        emit activeColorChanged();
    }
}
