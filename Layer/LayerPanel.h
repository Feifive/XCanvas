#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include "LayerManager.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

class LayerPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit LayerPanel(xcanvas::LayerManager* mgr, QWidget* parent = nullptr);
    ~LayerPanel() override = default;

  public slots:
    /**
     * @brief 全量刷新表格内容，保持与 LayerManager 的顺序一致
     */
    void refreshTable();

  private:
    void setupUI();
    void createConnections();

    // 单元格构建辅助函数
    void     setRowWidgets(int row, const xcanvas::LayerParameter& param);
    QWidget* createCheckBoxWidget(bool checked, int layerId, bool isVisibility);

  private slots:
    void onRowMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void onColorButtonClicked();
    void onModeChanged(int index);
    void onParamChanged(double value);
    void onSelectionChanged();

  private:
    enum Column
    {
        ColColor = 0,
        ColName,
        ColMode,
        ColSpeed,
        ColPower,
        ColVisible,
        ColOutput,
        Count
    };

    QTableWidget*          m_table;
    xcanvas::LayerManager* m_mgr;
};

#endif// LAYERPANEL_H
