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

    bool drawingToolLocked() const;
    void setDrawingToolLocked(bool locked);

    QString lastOpenedPath() const;
    QString lastOpenedPathOrDocumentsPath() const;
    void    setLastOpenedPath(const QString& path);

    QColor activeColor() const;
    void   setActiveColor(const QColor& color);

  signals:
    void settingsChanged();
    void gridContrastChanged();
    void drawingToolLockedChanged(bool locked);
    void activeColorChanged();

  private:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings();
    void    load();
    void    save();
    QString getConfigFilePath();

    GridContrast m_gridContrast      = GridContrast::Medium;
    bool         m_drawingToolLocked = false;
    QString      m_lastOpenedPath    = "";
    QColor       m_activeColor       = QColor("#000000");
};

#endif// APPSETTINGS_H
