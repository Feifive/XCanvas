#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class AppSettings : public QObject
{
    Q_OBJECT

  public:
    enum class GridContrast
    {
        Off = 0,
        Low,
        Medium,
        High
    };

  public:
    static AppSettings& instance();

    AppSettings(const AppSettings&)            = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    GridContrast gridContrast() const;
    void         setGridContrast(GridContrast contrast);

    QString lastOpenedPath() const;
    void    setLastOpenedPath(const QString& path);

  signals:
    void settingsChanged();
    void gridContrastChanged();

  private:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings();
    void    load();
    void    save();
    QString getConfigFilePath();

    GridContrast m_gridContrast   = GridContrast::Medium;
    QString      m_lastOpenedPath = "";
};

#endif// APPSETTINGS_H
