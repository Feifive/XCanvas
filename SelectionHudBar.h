#ifndef SELECTIONHUDBAR_H
#define SELECTIONHUDBAR_H

#include "SelectionSummary.h"
#include <qtfluentwidgets.h>
#include <QWidget>

class QDoubleSpinBox;
class QGridLayout;

class SelectionHudBar final : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionHudBar(QWidget *parent = nullptr);
    ~SelectionHudBar() override;
    void setSummary(const SelectionSummary& summary);
    bool keepAspectRatio() const;

    qfw::DoubleSpinBox* spinX() const     { return m_spinX; }
    qfw::DoubleSpinBox* spinY() const     { return m_spinY; }
    qfw::DoubleSpinBox* spinW() const     { return m_spinW; }
    qfw::DoubleSpinBox* spinH() const     { return m_spinH; }
    qfw::DoubleSpinBox* spinAngle() const { return m_spinAngle; }

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
    void angleEdited(double value);
    void keepAspectRatioToggled(bool enabled);

private:
    void hideGridColumn(int col, bool hide);
    void applyStyle();

private:
    QGridLayout* m_gridLayout;
    qfw::DoubleSpinBox* m_spinX     = nullptr;
    qfw::DoubleSpinBox* m_spinY     = nullptr;
    qfw::DoubleSpinBox* m_spinW     = nullptr;
    qfw::DoubleSpinBox* m_spinH     = nullptr;
    qfw::DoubleSpinBox* m_spinAngle = nullptr;
    qfw::TransparentToolButton* m_keepAspectRatioButton = nullptr;

    qfw::TransparentToolButton* m_booleanTool;
};
#endif
