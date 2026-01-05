#ifndef XSHADOWMENU_H
#define XSHADOWMENU_H

#include "XMenu.h"
#include <QMenu>
#include <QTimer>
class QToolButton;

class XHoverMenu final : public XMenu
{

    Q_OBJECT

  public:
    enum class PopupOrientation
    {
        Top,
        Bottom
    };

  public:
    explicit XHoverMenu(QToolButton* button, PopupOrientation orientation = PopupOrientation::Bottom, QWidget* parent = nullptr);
    ~XHoverMenu() override;
    void setGap(int gap);

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

  private slots:
    void onCloseTimeout();

  private:
    void showMenu();

  private:
    QToolButton*     m_button;
    QTimer           m_closeTimer;
    int              m_gap;
    PopupOrientation m_orientation;
};

#endif//XSHADOWMENU_H
