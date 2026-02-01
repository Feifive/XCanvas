#include "LayerPanel.h"
#include "CSwitchButton.h"
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QListView>
#include <QStyleFactory>

#include "Global.h"

LayerPanel::LayerPanel(xcanvas::LayerManager* mgr, QWidget* parent) : QWidget(parent), m_mgr(mgr)
{
    setObjectName("LayerPanel");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUI();
    createConnections();
    refreshTable();
    // setMinimumWidth(280);
    setFixedWidth(230);
}

void LayerPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 0, 1, 1);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(Column::Count);
    m_table->setHorizontalHeaderLabels({"图层", "模式", "速度/功率", "输出", "可见"});

    // 表头设置
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ColParams, QHeaderView::ResizeToContents);
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
    QSignalBlocker blocker(m_table);
    m_table->setRowCount(0);

    const QList<int>& order = m_mgr->layerOrder();
    for (const int id : order)
    {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        setRowWidgets(row, m_mgr->getLayer(id));
    }
    if (order.size() > 0) {
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table->horizontalHeader()->setSectionResizeMode(ColColor, QHeaderView::Stretch);
    }
}

void LayerPanel::setRowWidgets(int row, const xcanvas::LayerParameter& param)
{
    int id = param.id;

    auto* colorLabel = new QLabel();
    colorLabel->setText(QString("%1").arg(id, 2, 10, QChar('0')));
    colorLabel->setFixedSize(26, 20);
    colorLabel->setAlignment(Qt::AlignCenter);
    colorLabel->setProperty("layerId", id);

    colorLabel->setStyleSheet(QString(
        "QLabel { "
        "background-color: %1; "
        "color: %2; "
        "border: none; "
        "}")
        .arg(param.color.name())
        .arg(autoTextColor(param.color).name()));

    auto* colorContainer = new QWidget();
    auto* colorLayout    = new QHBoxLayout(colorContainer);
    colorLayout->addWidget(colorLabel);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setSpacing(2);
    colorLayout->setAlignment(Qt::AlignCenter);
    m_table->setCellWidget(row, ColColor, colorContainer);

    if (param.mode == xcanvas::ProcessMode::Image)
    {
        QTableWidgetItem* colModeItem = new QTableWidgetItem(tr("图像"));

        QFont font = colModeItem->font();
        font.setPixelSize(13);
        colModeItem->setFont(font);

        colModeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        colModeItem->setFlags(colModeItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, ColMode, colModeItem);
    }
    else
    {
        auto* modeCombo = new QComboBox();
        modeCombo->setStyle(QStyleFactory::create("Fusion"));
        modeCombo->setView(new QListView());
        modeCombo->addItems({ "切割", "扫描" });
        modeCombo->setCurrentIndex(static_cast<int>(param.mode));
        modeCombo->setProperty("layerId", id);
        connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LayerPanel::onModeChanged);
        m_table->setCellWidget(row, ColMode, modeCombo);
    }

    QTableWidgetItem* colParamsItem = new QTableWidgetItem(QString("%1/%2").arg(param.speed).arg(param.maxPower));
    colParamsItem->setTextAlignment(Qt::AlignCenter);
    colParamsItem->setFlags(colParamsItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, ColParams, colParamsItem);

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
            [this, layerId, isVisibility](const bool state)
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
        // int id = m_table->item(item->row(), ColName)->data(Qt::UserRole).toInt();
        // emit activeLayerChanged(id);
    }
}
