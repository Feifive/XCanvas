#ifndef HOVERMENUHELPER_H
#define HOVERMENUHELPER_H

#include <QObject>
#include <QPointer>
#include <QTimer>
class QToolButton;
class QMenu;

class HoverMenuHelper : public QObject
{
  public:
    enum class PopupOrientation
    {
        Top,
        Bottom
    };

    Q_OBJECT
  public:
    explicit HoverMenuHelper(QToolButton* btn, QMenu* menu, PopupOrientation = PopupOrientation::Bottom);
    void setGap(int gap);

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

  private slots:
    void onCloseTimeout();

  private:
    void showMenu();

  private:
    QPointer<QToolButton> m_button;
    QPointer<QMenu>       m_menu;
    QTimer                m_closeTimer;
    int                   m_gap;
    PopupOrientation      m_orientation;
};

#endif// HOVERMENUHELPER_H
