#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>

class QString;
class QGraphicsScene;
class MyGraphicsView;
class RulerWidget;
class QWidget;
class EditorSession;
namespace xcanvas
{
class LayerManager;
}

class CanvasWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit CanvasWidget(EditorSession* session, QWidget* parent = nullptr);
    xcanvas::LayerManager* layerManager();
    bool                   maybeSaveBeforeClose();
    QString                currentDocumentPath() const;
    bool                   openDocumentFile(const QString& path) const;

  signals:
    void documentDisplayNameChanged(const QString& name);

  private:
    void applyTheme();

  private:
    QGraphicsScene* m_pGraphicsScene;
    MyGraphicsView* m_pGraphicsView;
    RulerWidget*    m_pRulerHorizontal;
    RulerWidget*    m_pRulerVertical;
    QWidget*        m_pCornerWidget = nullptr;
};

#endif// CANVASWIDGET_H
