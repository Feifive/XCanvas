#include "LayerManager.h"

namespace xcanvas
{

LayerManager::LayerManager(QObject* parent) : QObject(parent)
{
    // 默认创建一个初始层
}

int LayerManager::findOrCreateLayerByColor(const QColor &color) {
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (it.value().color == color) {
            return it.key();
        }
    }

    const int newId = createLayer(color);

    return newId;
}

// 创建图层时，默认加到队列末尾
int LayerManager::createLayer(const QColor& color)
{
    const int id = m_nextId++;
    LayerParameter param;
    param.id    = id;
    param.color = color;

    m_layers[id] = param;
    m_layerOrder.append(id);// 新层默认最后加工

    emit layerAdded(id);
    emit orderChanged(m_layerOrder);
    return id;
}

void LayerManager::addShapeToLayer(Shape* shape)
{
    if (!shape) {
        return;
    }

    const int layerId = findOrCreateLayerByColor(shape->color());
    removeShapeFromLayer(shape);
    m_layers[layerId].shapes.insert(shape);
    shape->setLayerId(layerId);
}

void LayerManager::removeShapeFromLayer(Shape* shape)
{
    if (!shape) {
        return;
    }
    if (m_layers.contains(shape->layerId())) {
        m_layers[shape->layerId()].shapes.remove(shape);
    }
}

void LayerManager::setLayerColor(int layerId, const QColor& color)
{
    if (!m_layers.contains(layerId))
        return;

    m_layers[layerId].color = color;

    // 核心同步：遍历该层所有图元，更新视觉属性
    for (Shape* shape : m_layers[layerId].shapes)
    {
        shape->setColor(color);
        shape->setDirty(true);// 触发重绘
    }

    emit layerDataChanged(layerId);
}

void LayerManager::setLayerVisible(int layerId, bool visible)
{
    if (!m_layers.contains(layerId))
        return;
    m_layers[layerId].visible = visible;

    // CAM 逻辑：隐藏图层通常意味着在画布上不渲染
    for (Shape* shape : m_layers[layerId].shapes)
    {
        // 假设 Shape 有 setVisible 方法
        // shape->setVisible(visible);
    }
    emit layerVisibilityChanged(layerId, visible);
}

const QList<int> & LayerManager::layerOrder() const {
    return m_layerOrder;
}

// 移除图层时，同步移除顺序索引
void LayerManager::removeLayer(int layerId)
{
    if (m_layers.contains(layerId))
    {
        m_layers.remove(layerId);
        m_layerOrder.removeAll(layerId);
        emit layerRemoved(layerId);
        emit orderChanged(m_layerOrder);
    }
}

LayerParameter& LayerManager::getLayer(int layerId)
{
    return m_layers[layerId];
}

QList<int> LayerManager::layerIds() const {
    return m_layers.keys();
}

void LayerManager::moveLayer(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_layerOrder.size())
        return;
    if (toIndex < 0 || toIndex >= m_layerOrder.size())
        return;
    if (fromIndex == toIndex)
        return;

    int id = m_layerOrder.takeAt(fromIndex);
    m_layerOrder.insert(toIndex, id);

    emit orderChanged(m_layerOrder);
}

void LayerManager::moveUp(int layerId)
{
    int idx = m_layerOrder.indexOf(layerId);
    if (idx > 0)
    {
        moveLayer(idx, idx - 1);
    }
}

QList<LayerParameter*> LayerManager::getOrderedLayers()
{
    QList<LayerParameter*> orderedList;
    for (int id : m_layerOrder)
    {
        orderedList.append(&m_layers[id]);
    }
    return orderedList;
}

}// namespace xcanvas
