#ifndef TOOLGROUPBUTTON_H
#define TOOLGROUPBUTTON_H

#include "Global.h"
#include "XCanvasIcon.h"
#include <qtfluentwidgets.h>
#include <QTimer>
#include <QVector>

class QActionGroup;

class ToolGroupButton : public QWidget
{
    Q_OBJECT

public:
    struct ToolEntry
    {
        DrawingToolType  type;
        XCanvasIconType  iconType;
        QString          tooltip;
    };

    explicit ToolGroupButton(const QVector<ToolEntry>& tools, QWidget* parent = nullptr);
    ~ToolGroupButton() override;

    void             setCurrentTool(DrawingToolType type);
    DrawingToolType  currentTool() const { return m_currentTool; }

signals:
    void toolSelected(DrawingToolType type);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void createFlyout();
    void showFlyout();
    void hideFlyout();
    void updateMainButton();
    bool isMouseOverAny() const;

    QVector<ToolEntry>                    m_tools;
    qfw::TransparentToggleToolButton*     m_mainButton;
    qfw::CheckableMenu*                   m_flyout = nullptr;
    QActionGroup*                         m_flyoutActionGroup = nullptr;
    QVector<qfw::Action*>                 m_flyoutActions;
    DrawingToolType m_currentTool   = DrawingToolType::None;
    DrawingToolType m_lastUsedTool  = DrawingToolType::None;
    QTimer          m_showTimer;
    QTimer          m_hideTimer;
    bool            m_flyoutVisible = false;
};

#endif // TOOLGROUPBUTTON_H
