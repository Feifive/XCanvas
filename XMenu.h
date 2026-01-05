#ifndef XMENU_H
#define XMENU_H

#include <QMenu>
#include <QObject>

constexpr int blurRadius = 20;

class XMenu : public QMenu
{
    Q_OBJECT
  public:
    explicit XMenu(QWidget* parent = nullptr, bool needOffsetBlurRadius = true);
    explicit XMenu(const QString& title, QWidget* parent = nullptr, bool needOffsetBlurRadius = true);
    ~XMenu() override;

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    void initMenuStyle();

  private:
    bool m_needOffsetBlurRadius = true;
};

#endif// XMENU_H
