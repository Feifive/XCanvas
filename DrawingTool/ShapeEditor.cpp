#include "ShapeEditor.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Shape.h"
#include "ShapeVector.h"
#include "ReplaceCommand.h"
#include "MyMath.h"
#include "clipper2/clipper.h"

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
