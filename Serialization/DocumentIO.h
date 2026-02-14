#ifndef XCANVAS_SERIALIZATION_DOCUMENTIO_H
#define XCANVAS_SERIALIZATION_DOCUMENTIO_H

#include "../Layer/LayerManager.h"
#include "../Shape/ShapeManager.h"
#include <QRectF>

class QString;

namespace xcanvas
{
class Canvas;
}

namespace xcanvas::serialization
{
struct LayerSnapshot
{
    int         id       = -1;
    QColor      color;
    bool        visible  = true;
    bool        output   = true;
    ProcessMode mode     = ProcessMode::Cut;
    double      speed    = 100.0;
    double      minPower = 10.0;
    double      maxPower = 20.0;
};

struct LoadedDocument
{
    QRectF              canvasRect;
    QList<LayerSnapshot> layers;
    QList<int>          layerOrder;
    ShapeList           shapes;
};

void clearLoadedDocument(LoadedDocument* doc);
bool buildDocumentSnapshot(const Canvas* canvas, LoadedDocument* out, QString* err = nullptr);
bool readDocument(const QString& filePath, LoadedDocument* out, QString* err = nullptr);
bool writeDocument(const LoadedDocument& doc, const QString& filePath, QString* err = nullptr);
bool applyDocumentToCanvas(Canvas* canvas, LoadedDocument&& doc, QString* err = nullptr);

bool saveDocument(const Canvas* canvas, const QString& filePath, QString* err = nullptr);
bool loadDocument(Canvas* canvas, const QString& filePath, QString* err = nullptr);
}// namespace xcanvas::serialization

#endif// XCANVAS_SERIALIZATION_DOCUMENTIO_H
