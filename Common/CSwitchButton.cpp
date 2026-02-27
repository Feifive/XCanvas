//
// Created by Ze on 25-5-11.
//

#include "CSwitchButton.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

const qreal SliderMargin      = 3;
const qreal SliderMarginRatio = 0.03;

CSwitchButton::CSwitchButton(QWidget* parent) : QWidget(parent), m_iRadius(0), m_bOnOff(false)
{
    m_bEnabled     = true;
    m_backColorOn  = QColor("#6EB862");
    m_backColorOff = QColor("#E4E5E8");
    m_backColor    = m_backColorOn;
}

CSwitchButton::~CSwitchButton()
{
}

void CSwitchButton::SetSwitchOn(const bool bOn)
{
    // 初始化或强制设置时，直接赋值，不运行动画
    m_bOnOff    = bOn;
    m_backColor = m_bOnOff ? m_backColorOn : m_backColorOff;

    // 手动更新滑块位置
    const int   size   = qMin(width(), height());
    const qreal swidth = qMax(size - SliderMargin, SliderMargin);
    const qreal margin = (size - swidth) / 2;
    m_sliderLeftPos    = QPointF(margin, margin);
    m_sliderRightPos   = QPointF(this->width() - margin - swidth, margin);
    m_sliderRect.moveTo(m_bOnOff ? m_sliderRightPos : m_sliderLeftPos);

    update();
}

bool CSwitchButton::GetSwitchOn() const
{
    return m_bOnOff;
}

void CSwitchButton::Switch()
{
    SetSwitchStatus(!m_bOnOff);
}

void CSwitchButton::SetEnabled(bool enable)
{
    QWidget::setEnabled(enable);
    m_bEnabled = enable;

    update();
}

bool CSwitchButton::GetEnabled() const
{
    return m_bEnabled;
}

QColor CSwitchButton::GetBackColorOn() const
{
    return m_backColorOn;
}

QColor CSwitchButton::GetBackColorOff() const
{
    return m_backColorOff;
}

void CSwitchButton::SetBackColorOn(const QColor& color)
{
    m_backColorOn = color;
}

void CSwitchButton::SetBackColorOff(const QColor& color)
{
    m_backColorOff = color;
}

void CSwitchButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // 大椭圆
    painter.setBrush(m_backColor);
    painter.drawRoundedRect(rect(), m_iRadius, m_iRadius, Qt::AbsoluteSize);

    // 滑块
    painter.setBrush(Qt::white);
    painter.drawEllipse(m_sliderRect);

    // 绘制滑块阴影
    painter.setBrush(Qt::NoBrush);
    QColor    color(Qt::black);
    const int iCount = std::max(static_cast<int>((height() - m_sliderRect.height()) / 2), 0);
    if (iCount > 0)
    {
        const float stepColor = 0.15 / iCount;
        for (int i = m_sliderRect.height() / 2; i < height() / 2; i++)
        {
            float alpha = 0.15f - stepColor * (i - m_sliderRect.height() / 2);
            color.setAlphaF(std::max(0.0f, alpha));
            painter.setPen(color);
            painter.drawEllipse(m_sliderRect.center(), i, i);
        }
    }

    // 禁用状态覆盖层
    if (!m_bEnabled)
    {
        QColor disable(Qt::black);
        disable.setAlphaF(0.5);
        painter.setBrush(disable);
        painter.drawRoundedRect(rect(), m_iRadius, m_iRadius, Qt::AbsoluteSize);
    }
}

void CSwitchButton::resizeEvent(QResizeEvent* event)
{
    const int size = qMin(width(), height());
    m_iRadius      = size / 2;
    // const qreal width    = qMax(size - size * SliderMarginRatio, size * SliderMarginRatio);
    const qreal width  = qMax(size - SliderMargin, SliderMargin);
    const qreal margin = (size - width) / 2;
    m_sliderLeftPos    = QPointF(margin, margin);
    m_sliderRightPos   = QPointF(this->width() - margin - width, margin);
    m_sliderRect.setWidth(width);
    m_sliderRect.setHeight(width);
    m_sliderRect.moveTo(m_bOnOff ? m_sliderRightPos : m_sliderLeftPos);
    update();
}

void CSwitchButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (rect().contains(event->pos()))
    {
        Switch();
    }
}

QSize CSwitchButton::sizeHint() const
{
    return QSize(30, 15);
}

void CSwitchButton::SetSwitchStatus(bool bOnOff)
{
    m_bOnOff = bOnOff;
    emit SwitchChanged(m_bOnOff);

    // 颜色动画
    QPropertyAnimation* pColorAnimation = new QPropertyAnimation(this, "backColor");
    pColorAnimation->setDuration(200);
    pColorAnimation->setStartValue(m_backColor);
    pColorAnimation->setEndValue(m_bOnOff ? m_backColorOn : m_backColorOff);
    pColorAnimation->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);//停止后删除

    // 滑块动画
    QVariantAnimation* pPosAnimation = new QVariantAnimation(this);
    pPosAnimation->setDuration(200);
    pPosAnimation->setStartValue(m_sliderRect.topLeft());
    pPosAnimation->setEndValue(m_bOnOff ? m_sliderRightPos : m_sliderLeftPos);
    connect(pPosAnimation, &QPropertyAnimation::valueChanged,
            [=](const QVariant& value)
            {
                m_sliderRect.moveTo(value.toPoint());
                update();
            });
    pPosAnimation->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);//停止后删除
}
