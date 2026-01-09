#include "LayerManager.h"

namespace xcanvas
{

LayerManager::LayerManager(QObject* parent) : QObject(parent)
{
    // 默认创建一个初始层
    createLayer("Layer 0", Qt::black);
    createLayer("Layer 1", Qt::black);
}

// 创建图层时，默认加到队列末尾
int LayerManager::createLayer(const QString& name, const QColor& color)
{
    int            id = m_nextId++;
    LayerParameter param;
    param.id    = id;
    param.name  = name;
    param.color = color;

    m_layers[id] = param;
    m_layerOrder.append(id);// 新层默认最后加工

    emit layerAdded(id);
    emit orderChanged(m_layerOrder);
    return id;
}

void LayerManager::addShapeToLayer(int layerId, Shape* shape)
{
    if (!shape || !m_layers.contains(layerId))
        return;

    // 1. 如果 shape 原本在其他层，先移除
    removeShapeFromLayer(shape);

    // 2. 建立关系
    m_layers[layerId].shapes.insert(shape);

    // 3. 同步属性：让 Shape 的颜色跟随图层
    shape->setColor(m_layers[layerId].color);
    // 这里假设你在 Shape 中添加了 int m_layerId 成员
    // shape->setLayerId(layerId);
}

void LayerManager::removeShapeFromLayer(Shape* shape)
{
    if (!shape)
        return;
    // 遍历所有层寻找并移除（或者通过 shape->layerId() 快速定位）
    for (auto& layer : m_layers)
    {
        if (layer.shapes.contains(shape))
        {
            layer.shapes.remove(shape);
            break;
        }
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
