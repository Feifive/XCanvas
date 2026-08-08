#ifndef CLIPBOARDCOMMANDSERVICE_H
#define CLIPBOARDCOMMANDSERVICE_H

#include <functional>

#include <QPoint>
#include <QPointF>
#include <QStringList>
#include <QVector>

class ICanvasViewport;

namespace xcanvas
{
class Canvas;
class Shape;
}

class ClipboardCommandService final
{
  public:
    using RequestFullUpdate = std::function<void()>;
    using ImportFilesAt     = std::function<void(const QStringList&, const QPointF&)>;
    using ZoomAction        = std::function<void()>;

    ClipboardCommandService(ICanvasViewport* view, xcanvas::Canvas* canvas, RequestFullUpdate requestFullUpdate, ImportFilesAt importFilesAt);
    ~ClipboardCommandService();

    void clearCopiedShapes();
    void resetState();

    bool copySelectedShapes();
    bool cutSelectedShapes();
    bool pasteCopiedShapes();
    bool pasteCopiedShapesAt(const QPointF& scenePos);
    bool deleteSelectedShapes();
    bool groupSelectedShapes();
    bool ungroupSelectedShapes();
    bool pasteFromClipboard(const QPointF& scenePos);
    bool hasClipboardPasteContent() const;

    void showCanvasContextMenu(const QPoint& viewPos, const ZoomAction& onZoomIn, const ZoomAction& onZoomOut);

  private:
    ICanvasViewport*         m_view;
    xcanvas::Canvas*         m_canvas;
    RequestFullUpdate        m_requestFullUpdate;
    ImportFilesAt            m_importFilesAt;
    QVector<xcanvas::Shape*> m_copiedShapes;
    int                      m_pasteSerial;
};

#endif// CLIPBOARDCOMMANDSERVICE_H
