#include "LayerPanel.h"
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>

#include "Global.h"

namespace {
constexpr int kLayerRowHeight = 38;
}

LayerPanel::LayerPanel(xcanvas::LayerManager* mgr, QWidget* parent) : QWidget(parent), m_mgr(mgr)
{
    setObjectName("LayerPanel");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUI();
    createConnections();
    refreshTable();
    setMinimumWidth(340);
}

void LayerPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new qfw::TableWidget(this);
    m_table->setColumnCount(Column::Count);
    m_table->setHorizontalHeaderLabels({"图层", "模式", "速度/功率", "输出", "可见"});

    // 表头设置（固定关键列宽，避免 Fluent 控件被裁切）
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setMinimumSectionSize(48);
    m_table->horizontalHeader()->setSectionResizeMode(ColColor, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ColMode, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColParams, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColOutput, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColVisible, QHeaderView::Fixed);
    m_table->setColumnWidth(ColMode, 78);
    m_table->setColumnWidth(ColParams, 92);
    m_table->setColumnWidth(ColOutput, 58);
    m_table->setColumnWidth(ColVisible, 58);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->verticalHeader()->setMinimumSectionSize(34);
    m_table->setShowGrid(false);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setWordWrap(false);

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
        m_table->setRowHeight(row, kLayerRowHeight);
        setRowWidgets(row, m_mgr->getLayer(id));
    }

    // Hidden sentinel row: keep data rows away from being the physical last row.
    const int sentinelRow = m_table->rowCount();
    m_table->insertRow(sentinelRow);
    m_table->setRowHeight(sentinelRow, 1);
    m_table->setRowHidden(sentinelRow, true);
}

void LayerPanel::setRowWidgets(int row, const xcanvas::LayerParameter& param)
{
    int id = param.id;

    auto* colorLabel = new QLabel();
    // colorLabel->setText(QString("%1").arg(id, 2, 10, QChar('0')));
    colorLabel->setFixedSize(20, 20);
    colorLabel->setAlignment(Qt::AlignCenter);
    colorLabel->setProperty("layerId", id);

    colorLabel->setStyleSheet(QString(
        "QLabel { "
        "background-color: %1; "
        "color: %2; "
        "border: none; "
        "border-radius: 4px; "
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
        auto* modeCombo = new qfw::ComboBox();
        modeCombo->addItems({ "切割", "扫描" });
        modeCombo->setCurrentIndex(static_cast<int>(param.mode));
        modeCombo->setMinimumWidth(72);
        modeCombo->setFixedHeight(28);
        modeCombo->setProperty("layerId", id);
        connect(modeCombo, &qfw::ComboBox::currentIndexChanged, this, &LayerPanel::onModeChanged);
        m_table->setCellWidget(row, ColMode, createCenteredCellWidget(modeCombo));
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
    auto* cb        = new qfw::SwitchButton();
    cb->setText(QString());
    cb->setOnText(QString());
    cb->setOffText(QString());
    cb->setSpacing(0);

    // Initialize checked state without playing the slide animation.
    if (auto* indicator = cb->findChild<qfw::Indicator*>())
    {
        const QSignalBlocker blocker(indicator);
        indicator->setChecked(checked);
        indicator->setSliderX(checked ? 25 : 5);
    }
    else
    {
        cb->setChecked(checked);
    }

    connect(cb, &qfw::SwitchButton::checkedChanged, this,
            [this, layerId, isVisibility](const bool state)
            {
                if (isVisibility)
                    m_mgr->setLayerVisible(layerId, state);
                else
                    m_mgr->getLayer(layerId).output = state;
            });
    return createCenteredCellWidget(cb);
}

QWidget* LayerPanel::createCenteredCellWidget(QWidget* inner, const int hMargin, const int vMargin) const
{
    auto* container = new QWidget();
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    layout->setSpacing(0);
    layout->addWidget(inner, 0, Qt::AlignCenter);
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
