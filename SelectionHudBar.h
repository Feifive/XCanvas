#pragma once

#include <QWidget>

class QDoubleSpinBox;

/**
 * @brief 画布选中态悬浮工具栏（Selection HUD）
 * 仅包含：X / Y / W / H / Angle
 */
class SelectionHudBar final : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionHudBar(QWidget *parent = nullptr);
    ~SelectionHudBar() override;

    // ====== 对外访问接口（后续你绑定 Shape 时会用到） ======
    QDoubleSpinBox* spinX() const     { return m_spinX; }
    QDoubleSpinBox* spinY() const     { return m_spinY; }
    QDoubleSpinBox* spinW() const     { return m_spinW; }
    QDoubleSpinBox* spinH() const     { return m_spinH; }
    QDoubleSpinBox* spinAngle() const { return m_spinAngle; }

protected:
    void applyStyleSheet();

private:
    QDoubleSpinBox* m_spinX     = nullptr;
    QDoubleSpinBox* m_spinY     = nullptr;
    QDoubleSpinBox* m_spinW     = nullptr;
    QDoubleSpinBox* m_spinH     = nullptr;
    QDoubleSpinBox* m_spinAngle = nullptr;
};