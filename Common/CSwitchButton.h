//
// Created by Ze on 25-5-11.
//

#ifndef CSWITCHBUTTON_H
#define CSWITCHBUTTON_H

#include <QPushButton>
#include <QWidget>

class CSwitchButton : public QWidget
{

    Q_OBJECT
    Q_PROPERTY(QColor backColor MEMBER m_backColor)//新增背景颜色属性,用于动画
    Q_PROPERTY(bool enabled READ GetEnabled WRITE SetEnabled)
    Q_PROPERTY(QColor backColorOn WRITE SetBackColorOn)
    Q_PROPERTY(QColor backColorOff WRITE SetBackColorOff)

  public:
    explicit CSwitchButton(QWidget* parent = nullptr);
    ~CSwitchButton();

    void   SetSwitchOn(const bool bOn);
    bool   GetSwitchOn() const;
    void   Switch();
    void   SetEnabled(bool enable);
    bool   GetEnabled() const;
    QColor GetBackColor() const;
    void   SetBackColor(const QColor& color);
    void   SetBackColorOn(const QColor& color);
    void   SetBackColorOff(const QColor& color);

    void  paintEvent(QPaintEvent* event) override;
    void  resizeEvent(QResizeEvent* event) override;
    void  mouseReleaseEvent(QMouseEvent* event) override;
    QSize sizeHint() const override;

  signals:
    void SwitchChanged(bool enabled);

  private:
    void SetSwitchStatus(bool bOnOff);

  private:
    int     m_iRadius;
    QRectF  m_sliderRect;
    QPointF m_sliderLeftPos;
    QPointF m_sliderRightPos;
    QColor  m_backColor;
    QColor  m_backColorOn;
    QColor  m_backColorOff;
    bool    m_bOnOff;
    bool    m_bEnabled;
};

#endif//CSWITCHBUTTON_H
