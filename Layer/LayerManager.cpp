#include "LayerManager.h"

namespace xcanvas
{

LayerManager::LayerManager(QObject* parent) : QObject(parent)
{
    // 默认创建一个初始层
}

int LayerManager::findOrCreateLayerByShape(const Shape* shape) {
    const bool isImage = shape->isImage();
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (it.value().color == shape->color()) {
            if (isImage && it.value().mode == ProcessMode::Image)
            {
                return it.key();
            }
            if (!isImage && it.value().mode != ProcessMode::Image)
            {
                return it.key();
            }
        }
    }

    const int newId = createLayer(shape->color(), isImage);

    return newId;
}

int LayerManager::createLayer(const QColor& color, const bool isImage)
{
    const int id = m_nextId++;
    LayerParameter param;
    param.id    = id;
    param.color = color;
    param.mode  = isImage ? ProcessMode::Image : ProcessMode::Cut;

    m_layers[id] = param;
    m_layerOrder.append(id);

    emit orderChanged(m_layerOrder);
    return id;
}

bool LayerManager::createLayerWithId(const LayerParameter& layer)
{
    if (m_layers.contains(layer.id))
    {
        return false;
    }

    LayerParameter copy = layer;
    copy.shapes.clear();
    m_layers.insert(copy.id, copy);
    m_layerOrder.append(copy.id);

    if (copy.id >= m_nextId)
    {
        m_nextId = copy.id + 1;
    }

    emit orderChanged(m_layerOrder);
    return true;
}

void LayerManager::clearAllLayers()
{
    m_layers.clear();
    m_layerOrder.clear();
    m_nextId = 0;
    emit orderChanged(m_layerOrder);
}

void LayerManager::addShapeToLayer(Shape* shape)
{
    if (!shape) {
        return;
    }

    if (const int layerId = findOrCreateLayerByShape(shape); layerId != shape->layerId()) {
        removeShapeFromLayer(shape);
        m_layers[layerId].shapes.insert(shape);
        shape->setLayerId(layerId);
    }
}

void LayerManager::addShapesToLayer(ShapeList* shapes)
{
    if (!shapes) return;

    this->blockSignals(true);

    bool layerCreated = false;

    QMap<QPair<QRgb, bool>, int> localCache;

    for (Shape* shape : *shapes) {
        if (!shape) continue;

        int layerId = -1;

        if (QPair<QRgb, bool> key = {shape->color().rgba(), shape->isImage()}; localCache.contains(key)) {
            layerId = localCache[key];
        } else {
            layerId = findOrCreateLayerByShape(shape);
            localCache[key] = layerId;
            layerCreated = true;
        }

        if (shape->layerId() != layerId) {
            removeShapeFromLayer(shape);
            m_layers[layerId].shapes.insert(shape);
            shape->setLayerId(layerId);
        }
	}

    this->blockSignals(false);

    if (layerCreated) {
        emit orderChanged(m_layerOrder);
    }
}

void LayerManager::removeShapeFromLayer(Shape* shape)
{
    if (!shape || shape->layerId() == -1) {
        return;
    }

    const int layerId = shape->layerId();
    if (!m_layers.contains(layerId)) {
        return;
    }

    LayerParameter& param = m_layers[layerId];

    param.shapes.remove(shape);
    shape->setLayerId(-1);

    if (param.shapes.isEmpty()) {
        removeLayer(layerId);
    }
}

void LayerManager::removeShapesFromLayer(ShapeList* shapes)
{
    if (!shapes || shapes->isEmpty()) return;

    QSet<int> affectedLayerIds;

    for (Shape* shape : *shapes) {
        if (!shape) continue;

        if (int lid = shape->layerId(); m_layers.contains(lid)) {
            m_layers[lid].shapes.remove(shape);
            shape->setLayerId(-1);
            affectedLayerIds.insert(lid);
        }
    }

    bool hasChanged = false;
    for (int lid : affectedLayerIds) {
        if (m_layers[lid].shapes.isEmpty()) {
            m_layers.remove(lid);
            m_layerOrder.removeAll(lid);

            hasChanged = true;
        }
    }

    if (hasChanged) {
        emit orderChanged(m_layerOrder);
    }
}

QSet<Shape*> LayerManager::getShapesInLayer(const int layerId) const
{
    if (!m_layers.contains(layerId))
    {
        return {};
    }
    return m_layers[layerId].shapes;
}

int LayerManager::getLayerIdOfShape(Shape* shape) const
{
    if (!shape)
    {
        return -1;
    }
    return shape->layerId();
}

bool LayerManager::bindShapeToLayer(Shape* shape, const int layerId)
{
    if (!shape || !m_layers.contains(layerId))
    {
        return false;
    }

    if (shape->layerId() == layerId)
    {
        LayerParameter& layer = m_layers[layerId];
        layer.shapes.insert(shape);
        shape->setVisible(layer.visible);
        return true;
    }

    removeShapeFromLayer(shape);
    LayerParameter& layer = m_layers[layerId];
    layer.shapes.insert(shape);
    shape->setLayerId(layerId);
    shape->setVisible(layer.visible);
    return true;
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

void LayerManager::setLayerVisible(const int layerId, const bool visible)
{
    if (!m_layers.contains(layerId)) return;
    m_layers[layerId].visible = visible;

    for (Shape* shape : m_layers[layerId].shapes) {
        shape->setVisible(visible);
    }
    emit layerVisibilityChanged(layerId, visible);
}

const QList<int> & LayerManager::layerOrder() const {
    return m_layerOrder;
}

bool LayerManager::setLayerOrder(const QList<int>& order)
{
    if (order.size() != m_layers.size())
    {
        return false;
    }

    QSet<int> uniqueIds(order.begin(), order.end());
    if (uniqueIds.size() != order.size())
    {
        return false;
    }

    for (int id : order)
    {
        if (!m_layers.contains(id))
        {
            return false;
        }
    }

    m_layerOrder = order;
    emit orderChanged(m_layerOrder);
    return true;
}

void LayerManager::rebuildNextId()
{
    int maxId = -1;
    for (int id : std::as_const(m_layerOrder))
    {
        if (id > maxId)
        {
            maxId = id;
        }
    }
    for (auto it = m_layers.cbegin(); it != m_layers.cend(); ++it)
    {
        if (it.key() > maxId)
        {
            maxId = it.key();
        }
    }
    m_nextId = maxId + 1;
}

// 移除图层时，同步移除顺序索引
void LayerManager::removeLayer(int layerId)
{
    if (m_layers.contains(layerId))
    {
        m_layers.remove(layerId);
        m_layerOrder.removeAll(layerId);
        emit orderChanged(m_layerOrder);
    }
}

LayerParameter& LayerManager::getLayer(const int layerId)
{
    return m_layers[layerId];
}

const LayerParameter* LayerManager::tryGetLayer(const int layerId) const
{
    auto it = m_layers.constFind(layerId);
    if (it == m_layers.cend())
    {
        return nullptr;
    }
    return &it.value();
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
    const int idx = m_layerOrder.indexOf(layerId);
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
