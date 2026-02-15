#ifndef SELECTIONHUDBAR_H
#define SELECTIONHUDBAR_H

#include "SelectionSummary.h"
#include <QWidget>

class QDoubleSpinBox;
class QToolButton;
class QGridLayout;

class SelectionHudBar final : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionHudBar(QWidget *parent = nullptr);
    ~SelectionHudBar() override;
    void setSummary(const SelectionSummary& summary);
    bool keepAspectRatio() const;

    QDoubleSpinBox* spinX() const     { return m_spinX; }
    QDoubleSpinBox* spinY() const     { return m_spinY; }
    QDoubleSpinBox* spinW() const     { return m_spinW; }
    QDoubleSpinBox* spinH() const     { return m_spinH; }
    QDoubleSpinBox* spinAngle() const { return m_spinAngle; }

signals:
    void booleanUnion();
    void booleanIntersection();
    void booleanSubtractAB();
    void booleanSubtractBA();
    void mirrorHorizontal();
    void mirrorVertical();
    void alignLeft();
    void alignRight();
    void alignTop();
    void alignBottom();
    void alignHorizontalCenter();
    void alignVerticalCenter();
    void alignCenter();
    void xEdited(double value);
    void yEdited(double value);
    void wEdited(double value);
    void hEdited(double value);
    void keepAspectRatioToggled(bool enabled);

private:
    void hideGridColumn(int col, bool hide);

private:
    QGridLayout* m_gridLayout;
    QDoubleSpinBox* m_spinX     = nullptr;
    QDoubleSpinBox* m_spinY     = nullptr;
    QDoubleSpinBox* m_spinW     = nullptr;
    QDoubleSpinBox* m_spinH     = nullptr;
    QDoubleSpinBox* m_spinAngle = nullptr;
    QToolButton*    m_keepAspectRatioButton = nullptr;

    QToolButton* m_booleanTool;
};
#endif
