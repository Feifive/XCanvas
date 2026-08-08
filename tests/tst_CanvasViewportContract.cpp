#include "Canvas/ICanvasViewport.h"
#include "App/MyCanvasView.h"

#include <QGraphicsView>
#include <QtTest>
#include <type_traits>

static_assert(std::is_base_of_v<ICanvasViewport, MyCanvasView>);
static_assert(std::is_base_of_v<ICanvasNavigation, MyCanvasView>);
static_assert(std::is_base_of_v<xcanvas::CanvasView, MyCanvasView>);
static_assert(!std::is_base_of_v<QGraphicsView, MyCanvasView>);

class FakeViewport final : public ICanvasViewport
{
  public:
    QWidget* viewportWidget() const override { return nullptr; }
    QWidget* hostWidget() const override { return nullptr; }
    QRect viewportRect() const override { return QRect(0, 0, 800, 600); }
    QRect hostRect() const override { return QRect(0, 0, 820, 620); }
    QRectF worldRect() const override { return QRectF(-1000, -1000, 2000, 2000); }
    qreal zoomScale() const override { return 2.5; }
    QPointF mapToWorld(const QPointF& point) const override { return (point - QPointF(400, 300)) / zoomScale() + QPointF(10, 20); }
    QPointF mapFromWorld(const QPointF& point) const override { return (point - QPointF(10, 20)) * zoomScale() + QPointF(400, 300); }
    QRectF mapToWorld(const QRect& rect) const override { return QRectF(mapToWorld(rect.topLeft()), mapToWorld(rect.bottomRight())).normalized(); }
    QRectF mapFromWorld(const QRectF& rect) const override { return QRectF(mapFromWorld(rect.topLeft()), mapFromWorld(rect.bottomRight())).normalized(); }
    QPoint mapFromGlobalToView(const QPoint& point) const override { return point - QPoint(100, 200); }
    QPoint mapFromViewToGlobal(const QPoint& point) const override { return point + QPoint(100, 200); }
    void setInputMethodEnabled(bool) override {}
    void setViewCursor(const QCursor&) override {}
    Qt::CursorShape viewCursorShape() const override { return Qt::ArrowCursor; }
    void focusViewport() override {}
    void requestUpdate() const override {}
};

class CanvasViewportContractTest : public QObject
{
    Q_OBJECT
  private slots:
    void pointMappingRoundTrips()
    {
        const FakeViewport viewport;
        const QPointF worldPoint(-37.25, 98.5);
        const QPointF mappedBack = viewport.mapToWorld(viewport.mapFromWorld(worldPoint));
        QVERIFY(qAbs(mappedBack.x() - worldPoint.x()) < 1e-9);
        QVERIFY(qAbs(mappedBack.y() - worldPoint.y()) < 1e-9);
    }

    void viewportGeometryDoesNotIncludeHostChrome()
    {
        const FakeViewport viewport;
        QCOMPARE(viewport.viewportRect().size(), QSize(800, 600));
        QCOMPARE(viewport.hostRect().size(), QSize(820, 620));
        QCOMPARE(viewport.mapToWorld(viewport.viewportRect()).size(), QSizeF(319.6, 239.6));
    }

    void globalMappingRoundTrips()
    {
        const FakeViewport viewport;
        const QPoint point(123, 456);
        QCOMPARE(viewport.mapFromGlobalToView(viewport.mapFromViewToGlobal(point)), point);
    }
};

QTEST_APPLESS_MAIN(CanvasViewportContractTest)
#include "tst_CanvasViewportContract.moc"
