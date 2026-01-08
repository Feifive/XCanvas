#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include "Shape.h"
#include <QColor>
#include <QMap>
#include <QObject>
#include <QSet>

namespace xcanvas
{

// 加工模式
enum class ProcessMode
{
    Cut,
    Scan
};

// 图层工艺参数
struct LayerParameter
{
    int     id;
    QString name;
    QColor  color;
    bool    visible = true;
    bool    output  = true;

    ProcessMode mode     = ProcessMode::Cut;
    double      speed    = 100.0;
    double      minPower = 10.0;
    double      maxPower = 20;

    QSet<Shape*> shapes;// 该图层持有的所有图元引用
};

class LayerManager : public QObject
{
    Q_OBJECT
  public:
    explicit LayerManager(QObject* parent = nullptr);

    // --- 图层管理 ---
    int             createLayer(const QString& name, const QColor& color);
    void            removeLayer(int layerId);
    LayerParameter& getLayer(int layerId);
    QList<int>      layerIds() const
    {
        return m_layers.keys();
    }

    // --- 图元与图层的关系维护 ---
    void addShapeToLayer(int layerId, Shape* shape);
    void removeShapeFromLayer(Shape* shape);
    void moveShapeToLayer(int targetLayerId, Shape* shape);

    QSet<Shape*> getShapesInLayer(int layerId) const;
    int          getLayerIdOfShape(Shape* shape) const;

    // --- 批量属性同步 ---
    void setLayerColor(int layerId, const QColor& color);
    void setLayerVisible(int layerId, bool visible);

    // --- 顺序管理 ---
    const QList<int>& layerOrder() const
    {
        return m_layerOrder;
    }

    // 将图层从 fromIndex 移动到 toIndex (对应 UI 拖拽)
    void moveLayer(int fromIndex, int toIndex);

    // 上移/下移图层
    void moveUp(int layerId);
    void moveDown(int layerId);

    // 按顺序获取所有图层参数 (生成 G-Code 时使用)
    QList<LayerParameter*> getOrderedLayers();

  signals:
    void layerAdded(int id);
    void layerRemoved(int id);
    void layerDataChanged(int id);// 用于通知 QTableWidget 刷新某行
    void layerVisibilityChanged(int id, bool visible);
    void orderChanged(const QList<int>& newOrder);

  private:
    QMap<int, LayerParameter> m_layers;
    QList<int>                m_layerOrder;// 存储图层 ID 的顺序
    int                       m_nextId = 0;
};

}// namespace xcanvas

#endif// LAYERMANAGER_H
