#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include "Shape.h"
#include "ShapeManager.h"
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
    Scan,
    Image
};

// 图层工艺参数
struct LayerParameter
{
    int     id;
    QColor  color;
    bool    visible = true;
    bool    output  = true;

    ProcessMode mode     = ProcessMode::Cut;
    double      speed    = 100.0;
    double      minPower = 10.0;
    double      maxPower = 20;

    QSet<Shape*> shapes;
};

class LayerManager final : public QObject
{
    Q_OBJECT
  public:
    explicit LayerManager(QObject* parent = nullptr);

    int             findOrCreateLayerByShape(const Shape* shape);
    int             createLayer(const QColor& color, bool isImage);
    void            removeLayer(int layerId);
    LayerParameter& getLayer(int layerId);
    QList<int>      layerIds() const;

    // --- 图元与图层的关系维护 ---
    void addShapeToLayer(Shape* shape);
    void addShapesToLayer(ShapeList* shapes);
    void removeShapeFromLayer(Shape* shape);
	void removeShapesFromLayer(ShapeList* shapes);

    QSet<Shape*> getShapesInLayer(int layerId) const;
    int          getLayerIdOfShape(Shape* shape) const;

    // --- 批量属性同步 ---
    void setLayerColor(int layerId, const QColor& color);
    void setLayerVisible(int layerId, bool visible);

    // --- 顺序管理 ---
    const QList<int>& layerOrder() const;

    // 将图层从 fromIndex 移动到 toIndex (对应 UI 拖拽)
    void moveLayer(int fromIndex, int toIndex);

    // 上移/下移图层
    void moveUp(int layerId);
    void moveDown(int layerId);

    // 按顺序获取所有图层参数 (生成 G-Code 时使用)
    QList<LayerParameter*> getOrderedLayers();

signals:
    void orderChanged(const QList<int>& newOrder);
    void layerDataChanged(int id);
    void layerVisibilityChanged(int id, bool visible);

  private:
    QMap<int, LayerParameter> m_layers;
    QList<int>                m_layerOrder;// 存储图层 ID 的顺序
    int                       m_nextId = 0;
};

}// namespace xcanvas

#endif// LAYERMANAGER_H
