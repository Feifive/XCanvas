#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QColor>

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

    QColor activeColor() const;
    void   setActiveColor(const QColor& color);

  signals:
    void settingsChanged();
    void gridContrastChanged();
    void activeColorChanged();

  private:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings();
    void    load();
    void    save();
    QString getConfigFilePath();

    GridContrast m_gridContrast   = GridContrast::Medium;
    QString      m_lastOpenedPath = "";
    QColor       m_activeColor = QColor("#000000");
};

#endif// APPSETTINGS_H
