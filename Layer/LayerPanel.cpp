#include "LayerPanel.h"
#include "CSwitchButton.h"
#include <QColorDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

LayerPanel::LayerPanel(xcanvas::LayerManager* mgr, QWidget* parent) : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    createConnections();
    refreshTable();
    setMinimumWidth(315);
}

void LayerPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(Column::Count);
    m_table->setHorizontalHeaderLabels({"#", "层", "模式", "速度", "功率", "输出", "可见"});

    // 表头设置
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(ColName, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setFocusPolicy(Qt::NoFocus);

    // 开启拖拽排序
    m_table->verticalHeader()->setSectionsMovable(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(m_table);
}

void LayerPanel::createConnections()
{
    // 监听后端数据变化
    connect(m_mgr, &xcanvas::LayerManager::orderChanged, this, &LayerPanel::refreshTable);

    // 监听 UI 拖拽行移动
    connect(m_table->verticalHeader(), &QHeaderView::sectionMoved, this, &LayerPanel::onRowMoved);

    // 监听表格点击
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &LayerPanel::onSelectionChanged);
}

void LayerPanel::refreshTable()
{
    QSignalBlocker blocker(m_table);// 刷新时不触发 cellChanged 等信号
    m_table->setRowCount(0);

    const QList<int>& order = m_mgr->layerOrder();
    for (int id : order)
    {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        setRowWidgets(row, m_mgr->getLayer(id));
    }
}

void LayerPanel::setRowWidgets(int row, const xcanvas::LayerParameter& param)
{
    int id = param.id;

    // 1. 颜色按钮
    auto* colorBtn = new QPushButton();
    colorBtn->setFixedSize(24, 16);
    colorBtn->setProperty("layerId", id);
    colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #666;").arg(param.color.name()));
    connect(colorBtn, &QPushButton::clicked, this, &LayerPanel::onColorButtonClicked);

    auto* colorContainer = new QWidget();
    auto* colorLayout    = new QHBoxLayout(colorContainer);
    colorLayout->addWidget(colorBtn);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setAlignment(Qt::AlignCenter);
    m_table->setCellWidget(row, ColColor, colorContainer);

    // 2. 名称
    auto* nameItem = new QTableWidgetItem(param.name);
    nameItem->setData(Qt::UserRole, id);// 重要：将 ID 存入 Item
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, ColName, nameItem);

    // 3. 模式下拉框
    auto* modeCombo = new QComboBox();
    modeCombo->addItems({"切割", "扫描"});
    modeCombo->setCurrentIndex(static_cast<int>(param.mode));
    modeCombo->setProperty("layerId", id);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LayerPanel::onModeChanged);
    m_table->setCellWidget(row, ColMode, modeCombo);

    // 4. 速度
    auto* speedSpin = new QDoubleSpinBox();
    speedSpin->setButtonSymbols(QDoubleSpinBox::NoButtons);
    speedSpin->setRange(0.1, 5000.0);
    speedSpin->setValue(param.speed);
    speedSpin->setProperty("layerId", id);
    speedSpin->setProperty("type", "speed");
    connect(speedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LayerPanel::onParamChanged);
    m_table->setCellWidget(row, ColSpeed, speedSpin);

    // 5. 功率
    auto* powerSpin = new QDoubleSpinBox();
    powerSpin->setButtonSymbols(QDoubleSpinBox::NoButtons);
    powerSpin->setRange(0.0, 100.0);
    powerSpin->setValue(param.maxPower);
    powerSpin->setProperty("layerId", id);
    powerSpin->setProperty("type", "power");
    connect(powerSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LayerPanel::onParamChanged);
    m_table->setCellWidget(row, ColPower, powerSpin);

    // 6 & 7. 可见性与输出开关
    m_table->setCellWidget(row, ColVisible, createCheckBoxWidget(param.visible, id, true));
    m_table->setCellWidget(row, ColOutput, createCheckBoxWidget(param.output, id, false));
}

QWidget* LayerPanel::createCheckBoxWidget(bool checked, int layerId, bool isVisibility)
{
    auto* container = new QWidget();
    auto* layout    = new QHBoxLayout(container);
    auto* cb        = new CSwitchButton();
    cb->SetSwitchOn(checked);
    layout->addWidget(cb);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);
    connect(cb, &CSwitchButton::SwitchChanged,
            [this, layerId, isVisibility](bool state)
            {
                if (isVisibility)
                    m_mgr->setLayerVisible(layerId, state);
                else
                    m_mgr->getLayer(layerId).output = state;
            });
    return container;
}

// --- Slots ---

void LayerPanel::onRowMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    // 当 UI 拖动结束，通知后端更新数据顺序
    m_mgr->moveLayer(oldVisualIndex, newVisualIndex);
}

void LayerPanel::onColorButtonClicked()
{
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
        return;

    int    id       = btn->property("layerId").toInt();
    QColor newColor = QColorDialog::getColor(m_mgr->getLayer(id).color, this);
    if (newColor.isValid())
    {
        m_mgr->setLayerColor(id, newColor);
        btn->setStyleSheet(QString("background-color: %1; border: 1px solid #666;").arg(newColor.name()));
    }
}

void LayerPanel::onModeChanged(int index)
{
    int id                   = sender()->property("layerId").toInt();
    m_mgr->getLayer(id).mode = static_cast<xcanvas::ProcessMode>(index);
}

void LayerPanel::onParamChanged(double value)
{
    auto*   spin = qobject_cast<QDoubleSpinBox*>(sender());
    int     id   = spin->property("layerId").toInt();
    QString type = spin->property("type").toString();

    if (type == "speed")
        m_mgr->getLayer(id).speed = value;
    else if (type == "power")
        m_mgr->getLayer(id).maxPower = value;
}

void LayerPanel::onSelectionChanged()
{
    auto* item = m_table->currentItem();
    if (item)
    {
        // 可以根据选中的行，让 LayerManager 标记当前的“活动图层”
        int id = m_table->item(item->row(), ColName)->data(Qt::UserRole).toInt();
        // emit activeLayerChanged(id);
    }
}
