#include "ShapeEditor.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Shape.h"
#include "ShapeVector.h"
#include "ReplaceCommand.h"
#include "TransformCommand.h"
#include "MyMath.h"
#include "clipper2/clipper.h"
#include <map>
#include <limits>

using namespace Clipper2Lib;

xcanvas::ShapeEditor::ShapeEditor(MyGraphicsView *view, Canvas *canvas) : m_canvasView(view), m_canvas(canvas){
}

bool xcanvas::ShapeEditor::booleanUnion() const {
    constexpr double tolerance = 0.01;
    constexpr double scaleFactor = 1.0 / tolerance;
    QTransform scaleTransform;
    scaleTransform.scale(scaleFactor, scaleFactor);
    ShapeList clones;
    PathsD subject;
    for (auto* shape : m_canvas->shapeManager()->selectedShapeList()) {
        Shape* clone = shape->clone();
        clones.append(clone);
        auto* vectorShape = dynamic_cast<ShapeVector*>(shape);
        if (!vectorShape) {
            continue;
        }
        vectorShape->bakeTransform();
        QList<QPolygonF> polygons = vectorShape->path().toSubpathPolygons(scaleTransform);
        for (QPolygonF& polygon : polygons) {
            if (polygon.isEmpty()) {
                continue;
            }
            PathD path;
            path.reserve(polygon.size());
            for (QPointF& point : polygon) {
                point /= scaleFactor;
                path.emplace_back(point.x(), point.y());
            }
            subject.push_back(std::move(path));
        }
    }

    auto cleanup = qScopeGuard([&clones]() { qDeleteAll(clones); });

    if (subject.empty()) {
        return false;
    }

    PathsD solution;
    ClipperD clipper;
    clipper.AddSubject(subject);
    clipper.Execute(ClipType::Union, FillRule::NonZero, solution);

    if (solution.empty()) {
        return false;
    }

    QVector<Segment> resultSegments;
    for (const PathD& path : solution)
    {
        if (path.size() < 2) {
            continue;
        }

        resultSegments.push_back(Segment::moveTo(QPointF(path[0].x, path[0].y)));

        for (size_t i = 1; i < path.size(); ++i)
        {
            resultSegments.push_back(Segment::lineTo(QPointF(path[i].x, path[i].y)));
        }

        resultSegments.push_back(Segment::lineTo(QPointF(path[0].x, path[0].y)));
    }

    QVector<Segment> optimizedSegments = geometryMath::Optimize(resultSegments);

    auto* resultShape = new ShapeVector;
    resultShape->setSemantic(VectorSemantic::Complex);
    resultShape->setSegments(std::move(optimizedSegments));
    m_canvas->undoStack()->push(new ReplaceShapesCommand(m_canvas->shapeManager(), m_canvas->layerManager(), m_canvas->shapeManager()->selectedShapeList(), {resultShape}));
    m_canvas->shapeManager()->selectShape(resultShape, true);
    m_canvasView->requestFullUpdate();

    return true;
}

bool xcanvas::ShapeEditor::booleanIntersection() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    constexpr double tolerance = 0.01;
    constexpr double scaleFactor = 1.0 / tolerance;
    QTransform scaleTransform;
    scaleTransform.scale(scaleFactor, scaleFactor);
    ShapeList clones;

    PathsD subject;
    auto* firstShape = dynamic_cast<ShapeVector*>(selectedShapes[0]);
    if (!firstShape) {
        return false;
    }
    clones.append(firstShape->clone());
    firstShape->bakeTransform();
    QList<QPolygonF> polygonsFirst = firstShape->path().toSubpathPolygons(scaleTransform);
    for (QPolygonF& polygon : polygonsFirst) {
        if (polygon.isEmpty()) {
            continue;
        }
        PathD path;
        path.reserve(polygon.size());
        for (QPointF& point : polygon) {
            point /= scaleFactor;
            path.emplace_back(point.x(), point.y());
        }
        subject.push_back(std::move(path));
    }

    PathsD clip;
    for (int i = 1; i < selectedShapes.size(); ++i) {
        auto* shape = dynamic_cast<ShapeVector*>(selectedShapes[i]);
        if (!shape) {
            continue;
        }
        clones.append(shape->clone());
        shape->bakeTransform();
        QList<QPolygonF> polygons = shape->path().toSubpathPolygons(scaleTransform);
        for (QPolygonF& polygon : polygons) {
            if (polygon.isEmpty()) {
                continue;
            }
            PathD path;
            path.reserve(polygon.size());
            for (QPointF& point : polygon) {
                point /= scaleFactor;
                path.emplace_back(point.x(), point.y());
            }
            clip.push_back(std::move(path));
        }
    }

    auto cleanup = qScopeGuard([&clones]() { qDeleteAll(clones); });

    if (subject.empty() || clip.empty()) {
        return false;
    }

    PathsD solution;
    ClipperD clipper;
    clipper.AddSubject(subject);
    clipper.AddClip(clip);
    clipper.Execute(ClipType::Intersection, FillRule::NonZero, solution);

    if (solution.empty()) {
        return false;
    }

    QVector<Segment> resultSegments;
    for (const PathD& path : solution)
    {
        if (path.size() < 2) {
            continue;
        }

        resultSegments.push_back(Segment::moveTo(QPointF(path[0].x, path[0].y)));

        for (size_t i = 1; i < path.size(); ++i)
        {
            resultSegments.push_back(Segment::lineTo(QPointF(path[i].x, path[i].y)));
        }

        resultSegments.push_back(Segment::lineTo(QPointF(path[0].x, path[0].y)));
    }

    QVector<Segment> optimizedSegments = geometryMath::Optimize(resultSegments);

    auto* resultShape = new ShapeVector;
    resultShape->setSemantic(VectorSemantic::Complex);
    resultShape->setSegments(std::move(optimizedSegments));
    m_canvas->undoStack()->push(new ReplaceShapesCommand(m_canvas->shapeManager(), m_canvas->layerManager(), m_canvas->shapeManager()->selectedShapeList(), {resultShape}));
    m_canvas->shapeManager()->selectShape(resultShape, true);
    m_canvasView->requestFullUpdate();

    return true;
}

bool xcanvas::ShapeEditor::booleanSubtractAB() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.size() != 2) {
        return false;
    }

    constexpr double tolerance = 0.01;
    constexpr double scaleFactor = 1.0 / tolerance;
    QTransform scaleTransform;
    scaleTransform.scale(scaleFactor, scaleFactor);

    PathsD subject;
    auto* shapeA = dynamic_cast<ShapeVector*>(selectedShapes[0]);
    if (!shapeA) return false;
    shapeA->bakeTransform();
    QList<QPolygonF> polygonsA = shapeA->path().toSubpathPolygons(scaleTransform);
    for (QPolygonF& polygon : polygonsA) {
        if (polygon.isEmpty()) continue;
        PathD path;
        path.reserve(polygon.size());
        for (QPointF& point : polygon) {
            point /= scaleFactor;
            path.emplace_back(point.x(), point.y());
        }
        subject.push_back(std::move(path));
    }

    PathsD clip;
    auto* shapeB = dynamic_cast<ShapeVector*>(selectedShapes[1]);
    if (!shapeB) return false;
    shapeB->bakeTransform();
    QList<QPolygonF> polygonsB = shapeB->path().toSubpathPolygons(scaleTransform);
    for (QPolygonF& polygon : polygonsB) {
        if (polygon.isEmpty()) continue;
        PathD path;
        path.reserve(polygon.size());
        for (QPointF& point : polygon) {
            point /= scaleFactor;
            path.emplace_back(point.x(), point.y());
        }
        clip.push_back(std::move(path));
    }

    if (subject.empty() || clip.empty()) {
        return false;
    }

    PathsD solution;
    ClipperD clipper;
    clipper.AddSubject(subject);
    clipper.AddClip(clip);
    clipper.Execute(ClipType::Difference, FillRule::NonZero, solution);

    if (solution.empty()) {
        return false;
    }

    QVector<Segment> resultSegments;
    for (const PathD& path : solution)
    {
        if (path.size() < 2) continue;
        resultSegments.push_back(Segment::moveTo(QPointF(path[0].x, path[0].y)));
        for (size_t i = 1; i < path.size(); ++i) {
            resultSegments.push_back(Segment::lineTo(QPointF(path[i].x, path[i].y)));
        }
        resultSegments.push_back(Segment::lineTo(QPointF(path[0].x, path[0].y)));
    }

    QVector<Segment> optimizedSegments = geometryMath::Optimize(resultSegments);
    auto* resultShape = new ShapeVector;
    resultShape->setSemantic(VectorSemantic::Complex);
    resultShape->setSegments(std::move(optimizedSegments));
    m_canvas->undoStack()->push(new ReplaceShapesCommand(m_canvas->shapeManager(), m_canvas->layerManager(), m_canvas->shapeManager()->selectedShapeList(), {resultShape}));
    m_canvas->shapeManager()->selectShape(resultShape, true);
    m_canvasView->requestFullUpdate();

    return true;
}

bool xcanvas::ShapeEditor::booleanSubtractBA() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.size() != 2) {
        return false;
    }

    constexpr double tolerance = 0.01;
    constexpr double scaleFactor = 1.0 / tolerance;
    QTransform scaleTransform;
    scaleTransform.scale(scaleFactor, scaleFactor);

    PathsD subject;
    auto* shapeB = dynamic_cast<ShapeVector*>(selectedShapes[1]);
    if (!shapeB) return false;
    shapeB->bakeTransform();
    QList<QPolygonF> polygonsB = shapeB->path().toSubpathPolygons(scaleTransform);
    for (QPolygonF& polygon : polygonsB) {
        if (polygon.isEmpty()) continue;
        PathD path;
        path.reserve(polygon.size());
        for (QPointF& point : polygon) {
            point /= scaleFactor;
            path.emplace_back(point.x(), point.y());
        }
        subject.push_back(std::move(path));
    }

    PathsD clip;
    auto* shapeA = dynamic_cast<ShapeVector*>(selectedShapes[0]);
    if (!shapeA) return false;
    shapeA->bakeTransform();
    QList<QPolygonF> polygonsA = shapeA->path().toSubpathPolygons(scaleTransform);
    for (QPolygonF& polygon : polygonsA) {
        if (polygon.isEmpty()) continue;
        PathD path;
        path.reserve(polygon.size());
        for (QPointF& point : polygon) {
            point /= scaleFactor;
            path.emplace_back(point.x(), point.y());
        }
        clip.push_back(std::move(path));
    }

    if (subject.empty() || clip.empty()) {
        return false;
    }

    PathsD solution;
    ClipperD clipper;
    clipper.AddSubject(subject);
    clipper.AddClip(clip);
    clipper.Execute(ClipType::Difference, FillRule::NonZero, solution);

    if (solution.empty()) {
        return false;
    }

    QVector<Segment> resultSegments;
    for (const PathD& path : solution)
    {
        if (path.size() < 2) continue;
        resultSegments.push_back(Segment::moveTo(QPointF(path[0].x, path[0].y)));
        for (size_t i = 1; i < path.size(); ++i) {
            resultSegments.push_back(Segment::lineTo(QPointF(path[i].x, path[i].y)));
        }
        resultSegments.push_back(Segment::lineTo(QPointF(path[0].x, path[0].y)));
    }

    QVector<Segment> optimizedSegments = geometryMath::Optimize(resultSegments);
    auto* resultShape = new ShapeVector;
    resultShape->setSemantic(VectorSemantic::Complex);
    resultShape->setSegments(std::move(optimizedSegments));
    m_canvas->undoStack()->push(new ReplaceShapesCommand(m_canvas->shapeManager(), m_canvas->layerManager(), m_canvas->shapeManager()->selectedShapeList(), {resultShape}));
    m_canvas->shapeManager()->selectShape(resultShape, true);
    m_canvasView->requestFullUpdate();

    return true;
}

bool xcanvas::ShapeEditor::mirrorHorizontal() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 获取选中形状的整体边界矩形
    const QRectF overallBounds = m_canvas->shapeManager()->selectedBoundingRect();
    if (overallBounds.isNull()) {
        return false;
    }

    QPointF mirrorCenter = overallBounds.center();

    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        shape->scale(-1.0, 1.0, mirrorCenter);
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Mirror Horizontal"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::mirrorVertical() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 获取选中形状的整体边界矩形
    const QRectF overallBounds = m_canvas->shapeManager()->selectedBoundingRect();
    if (overallBounds.isNull()) {
        return false;
    }

    QPointF mirrorCenter = overallBounds.center();

    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        shape->scale(1.0, -1.0, mirrorCenter);
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Mirror Vertical"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignLeft() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标X坐标
    qreal targetX;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布左边界
        targetX = m_canvas->canvasRect().left();
    } else {
        // 多选：对齐到最左侧形状的左边界
        targetX = std::numeric_limits<qreal>::max();
        for (const auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            targetX = qMin(targetX, bounds.left());
        }
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetX = targetX - bounds.left();
        shape->translate(QPointF(offsetX, 0));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Left"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignRight() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标X坐标
    qreal targetX;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布右边界
        targetX = m_canvas->canvasRect().right();
    } else {
        // 多选：对齐到最右侧形状的右边界
        targetX = std::numeric_limits<qreal>::lowest();
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            targetX = qMax(targetX, bounds.right());
        }
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetX = targetX - bounds.right();
        shape->translate(QPointF(offsetX, 0));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Right"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignTop() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标Y坐标
    qreal targetY;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布顶部边界
        targetY = m_canvas->canvasRect().top();
    } else {
        // 多选：对齐到最顶部形状的顶部边界
        targetY = std::numeric_limits<qreal>::max();
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            targetY = qMin(targetY, bounds.top());
        }
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetY = targetY - bounds.top();
        shape->translate(QPointF(0, offsetY));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Top"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignBottom() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标Y坐标
    qreal targetY;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布底部边界
        targetY = m_canvas->canvasRect().bottom();
    } else {
        // 多选：对齐到最底部形状的底部边界
        targetY = std::numeric_limits<qreal>::lowest();
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            targetY = qMax(targetY, bounds.bottom());
        }
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetY = targetY - bounds.bottom();
        shape->translate(QPointF(0, offsetY));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Bottom"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignHorizontalCenter() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标X坐标（水平中心）
    qreal targetX;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布水平中心
        targetX = m_canvas->canvasRect().center().x();
    } else {
        // 多选：对齐到所有形状的水平中心平均值
        qreal sumX = 0;
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            sumX += bounds.center().x();
        }
        targetX = sumX / selectedShapes.size();
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetX = targetX - bounds.center().x();
        shape->translate(QPointF(offsetX, 0));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Horizontal Center"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignVerticalCenter() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标Y坐标（垂直中心）
    qreal targetY;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布垂直中心
        targetY = m_canvas->canvasRect().center().y();
    } else {
        // 多选：对齐到所有形状的垂直中心平均值
        qreal sumY = 0;
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            sumY += bounds.center().y();
        }
        targetY = sumY / selectedShapes.size();
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        qreal offsetY = targetY - bounds.center().y();
        shape->translate(QPointF(0, offsetY));
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Vertical Center"));
    m_canvasView->requestFullUpdate();
    return true;
}

bool xcanvas::ShapeEditor::alignCenter() const {
    const auto& selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.empty()) {
        return false;
    }

    // 计算目标中心点
    QPointF targetCenter;
    if (selectedShapes.size() == 1) {
        // 单选：对齐到画布中心
        targetCenter = m_canvas->canvasRect().center();
    } else {
        // 多选：对齐到所有形状的中心点平均值
        qreal sumX = 0, sumY = 0;
        for (auto* shape : selectedShapes) {
            QRectF bounds = shape->boundingRect();
            sumX += bounds.center().x();
            sumY += bounds.center().y();
        }
        targetCenter = QPointF(sumX / selectedShapes.size(),
                              sumY / selectedShapes.size());
    }

    // 应用平移
    std::map<Shape*, QTransform> beforeTransform;
    for (auto* shape : selectedShapes) {
        beforeTransform[shape] = shape->transform();
        QRectF bounds = shape->boundingRect();
        QPointF currentCenter = bounds.center();
        QPointF offset = targetCenter - currentCenter;
        shape->translate(offset);
    }

    m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), beforeTransform, "Align Center"));
    m_canvasView->requestFullUpdate();
    return true;
}
