#ifndef VIEWINTERACTIONCONTROLLER_H
#define VIEWINTERACTIONCONTROLLER_H

#include <functional>

#include <QPoint>
#include <QPointF>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QGraphicsView;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

class ViewInteractionController final
{
  public:
    using BoolAction        = std::function<bool()>;
    using VoidAction        = std::function<void()>;
    using ZoomAtAction      = std::function<void(const QPointF&)>;
    using ShowMenuAction    = std::function<void(const QPoint&)>;
    using IsProjectPath     = std::function<bool(const QString&)>;
    using OpenDocumentFile  = std::function<bool(const QString&)>;
    using ImportFilesAt     = std::function<void(const QStringList&, const QPointF&)>;

    ViewInteractionController(
        QGraphicsView*  view,
        BoolAction      isSelectTool,
        ShowMenuAction  showCanvasContextMenu,
        BoolAction      groupSelectedShapes,
        BoolAction      ungroupSelectedShapes,
        BoolAction      cutSelectedShapes,
        BoolAction      copySelectedShapes,
        BoolAction      pasteCopiedShapes,
        VoidAction      selectAllAction,
        ZoomAtAction    zoomInAction,
        ZoomAtAction    zoomOutAction,
        IsProjectPath   isProjectFilePath,
        BoolAction      maybeSaveBeforeProceed,
        OpenDocumentFile openDocumentFile,
        ImportFilesAt   importFilesAt);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    bool mouseReleaseEvent(QMouseEvent* event);
    bool keyPressEvent(QKeyEvent* event);
    bool wheelEvent(QWheelEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

  private:
    QGraphicsView* m_view;

    QPointF m_startPos;
    bool    m_dragging;
    QPoint  m_rightPressPos;
    bool    m_rightDragged;

    BoolAction        m_isSelectTool;
    ShowMenuAction    m_showCanvasContextMenu;
    BoolAction        m_groupSelectedShapes;
    BoolAction        m_ungroupSelectedShapes;
    BoolAction        m_cutSelectedShapes;
    BoolAction        m_copySelectedShapes;
    BoolAction        m_pasteCopiedShapes;
    VoidAction        m_selectAllAction;
    ZoomAtAction      m_zoomInAction;
    ZoomAtAction      m_zoomOutAction;
    IsProjectPath     m_isProjectFilePath;
    BoolAction        m_maybeSaveBeforeProceed;
    OpenDocumentFile  m_openDocumentFile;
    ImportFilesAt     m_importFilesAt;
};

#endif// VIEWINTERACTIONCONTROLLER_H
