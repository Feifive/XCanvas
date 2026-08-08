#include "Canvas/CanvasView.h"
#include "Canvas/RulerRenderer.h"
#include "Canvas/RulerInteractionPolicy.h"
#include "Canvas/SelectionOutlineStyle.h"

#include <QImage>
#include <QApplication>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QtTest>

#include <type_traits>

static_assert(std::is_base_of_v<QAbstractScrollArea, xcanvas::CanvasView>);
static_assert(std::is_base_of_v<ICanvasViewport, xcanvas::CanvasView>);
static_assert(std::is_base_of_v<ICanvasNavigation, xcanvas::CanvasView>);

namespace
{
bool fuzzyPoint(const QPointF& actual, const QPointF& expected, const qreal epsilon = 1e-8)
{
    return qAbs(actual.x() - expected.x()) <= epsilon
        && qAbs(actual.y() - expected.y()) <= epsilon;
}

class InputProbeCanvasView final : public xcanvas::CanvasView
{
  public:
    bool keyReceived = false;
    bool inputMethodReceived = false;
    bool worldDrawn = false;
    QPainter::RenderHints worldHints;

  protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        focusForPointerPress();
        if (!filterRulerMousePress(event))
        {
            event->accept();
        }
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        keyReceived = true;
        event->accept();
    }

    void inputMethodEvent(QInputMethodEvent* event) override
    {
        inputMethodReceived = true;
        event->accept();
    }

    void drawWorld(QPainter& painter, const QRectF&) override
    {
        worldDrawn = true;
        worldHints = painter.renderHints();
    }
};
}// namespace

class tst_CanvasView final : public QObject
{
    Q_OBJECT
  private slots:
    void mappingUsesEntireViewport();
    void cameraAndScrollBarsSynchronize();
    void rulerHitAreasDoNotOffsetMapping();
    void rulerBackgroundIsSemitransparent();
    void rulerLabelsUseWorldRectCenterAsOrigin();
    void rulerMovePolicyPreservesCanvasDrag();
    void hostOwnsFocusAndReceivesInput();
    void worldPainterUsesExpectedRenderHints();
    void selectionOutlineUsesShapeColorAndAnimatedDash();
};

void tst_CanvasView::mappingUsesEntireViewport()
{
    xcanvas::CanvasView view;
    view.resize(640, 480);
    view.show();
    QCoreApplication::processEvents();

    view.centerOnWorld(QPointF(10000.0, 10000.0));
    QVERIFY(view.setZoomScale(2.0));

    const QPointF viewportCenter(view.viewportRect().width() / 2.0,
                                 view.viewportRect().height() / 2.0);
    QVERIFY(fuzzyPoint(view.mapToWorld(viewportCenter), QPointF(10000.0, 10000.0)));

    const QPointF underRuler(5.0, 5.0);
    QVERIFY(fuzzyPoint(view.mapFromWorld(view.mapToWorld(underRuler)), underRuler));
}

void tst_CanvasView::cameraAndScrollBarsSynchronize()
{
    xcanvas::CanvasView view;
    view.resize(500, 400);
    view.show();
    QCoreApplication::processEvents();
    view.setWorldRect(QRectF(0.0, 0.0, 20000.0, 20000.0));
    view.setZoomScale(2.0);
    view.centerOnWorld(QPointF(4000.25, 3000.75));

    QCOMPARE(view.horizontalScrollBar()->value(),
             qRound(4000.25 * 2.0 - view.viewportRect().width() / 2.0));
    QCOMPARE(view.verticalScrollBar()->value(),
             qRound(3000.75 * 2.0 - view.viewportRect().height() / 2.0));
    QVERIFY(fuzzyPoint(view.camera().center(), QPointF(4000.25, 3000.75)));

    view.horizontalScrollBar()->setValue(1200);
    const qreal expectedCenterX = (1200.0 + view.viewportRect().width() / 2.0) / 2.0;
    QVERIFY(qAbs(view.camera().center().x() - expectedCenterX) < 1e-8);
}

void tst_CanvasView::rulerHitAreasDoNotOffsetMapping()
{
    xcanvas::CanvasView view;
    view.resize(500, 400);
    view.show();
    QCoreApplication::processEvents();
    view.centerOnWorld(QPointF(10000.0, 10000.0));

    QCOMPARE(view.rulerHitArea(QPoint(2, 2)), xcanvas::RulerRenderer::HitArea::Corner);
    QCOMPARE(view.rulerHitArea(QPoint(100, 2)), xcanvas::RulerRenderer::HitArea::Horizontal);
    QCOMPARE(view.rulerHitArea(QPoint(2, 100)), xcanvas::RulerRenderer::HitArea::Vertical);
    QCOMPARE(view.rulerHitArea(QPoint(100, 100)), xcanvas::RulerRenderer::HitArea::None);

    const QPointF atOrigin = view.mapToWorld(QPointF(0.0, 0.0));
    const QPointF expected = view.camera().mapToWorld(QPointF(0.0, 0.0), view.viewportRect().size());
    QVERIFY(fuzzyPoint(atOrigin, expected));
}

void tst_CanvasView::rulerBackgroundIsSemitransparent()
{
    constexpr int width = 160;
    constexpr int height = 120;
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    xcanvas::CanvasCamera camera(1.0, QPointF(80.0, 60.0));
    xcanvas::RulerRenderer renderer;
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(40, 50, 60));
    palette.setColor(QPalette::Window, QColor(140, 150, 160));
    palette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    const QColor rulerBackground = renderer.backgroundColor(palette.color(QPalette::Base));
    QCOMPARE(rulerBackground.red(), 40);
    QCOMPARE(rulerBackground.green(), 50);
    QCOMPARE(rulerBackground.blue(), 60);
    QCOMPARE(rulerBackground.alpha(), 204);
    {
        QPainter painter(&image);
        renderer.draw(&painter, image.rect(), image.size(), camera,
                      QPointF(width / 2.0, height / 2.0), palette.color(QPalette::Base), palette);
    }

    const QColor horizontalPixel = image.pixelColor(100, 20);
    const QColor verticalPixel = image.pixelColor(20, 100);
    const QColor canvasPixel = image.pixelColor(100, 100);
    QVERIFY(horizontalPixel.alpha() > 0 && horizontalPixel.alpha() < 255);
    QVERIFY(verticalPixel.alpha() > 0 && verticalPixel.alpha() < 255);
    QCOMPARE(canvasPixel.alpha(), 0);
}

void tst_CanvasView::rulerLabelsUseWorldRectCenterAsOrigin()
{
    QCOMPARE(xcanvas::RulerRenderer::labelText(10000.0, 10000.0), QStringLiteral("0"));
    QCOMPARE(xcanvas::RulerRenderer::labelText(9750.0, 10000.0), QStringLiteral("-250"));
    QCOMPARE(xcanvas::RulerRenderer::labelText(10250.0, 10000.0), QStringLiteral("250"));
}

void tst_CanvasView::rulerMovePolicyPreservesCanvasDrag()
{
    using xcanvas::RulerInteractionPolicy;

    QVERIFY(RulerInteractionPolicy::blocksMove(true, true, Qt::LeftButton));
    QVERIFY(RulerInteractionPolicy::blocksMove(true, false, Qt::LeftButton));
    QVERIFY(!RulerInteractionPolicy::blocksMove(false, true, Qt::LeftButton));
    QVERIFY(RulerInteractionPolicy::blocksMove(false, true, Qt::NoButton));
    QVERIFY(!RulerInteractionPolicy::blocksMove(false, false, Qt::NoButton));
}

void tst_CanvasView::hostOwnsFocusAndReceivesInput()
{
    InputProbeCanvasView view;
    view.resize(320, 240);
    view.show();
    view.activateWindow();
    view.focusViewport();
    QTRY_COMPARE(QApplication::focusWidget(), static_cast<QWidget*>(&view));
    QCOMPARE(view.viewport()->focusPolicy(), Qt::NoFocus);
    view.setInputMethodEnabled(false);
    QVERIFY(!view.testAttribute(Qt::WA_InputMethodEnabled));
    QVERIFY(!view.viewport()->testAttribute(Qt::WA_InputMethodEnabled));
    view.setInputMethodEnabled(true);
    QVERIFY(view.testAttribute(Qt::WA_InputMethodEnabled));
    QVERIFY(!view.viewport()->testAttribute(Qt::WA_InputMethodEnabled));

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QStringLiteral("a"));
    QApplication::sendEvent(QApplication::focusWidget(), &keyEvent);
    QVERIFY(view.keyReceived);

    QInputMethodEvent inputMethodEvent(QStringLiteral("preedit"), {});
    QApplication::sendEvent(QApplication::focusWidget(), &inputMethodEvent);
    QVERIFY(view.inputMethodReceived);

    view.clearFocus();
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    QTRY_COMPARE(QApplication::focusWidget(), static_cast<QWidget*>(&view));

    view.clearFocus();
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QTRY_COMPARE(QApplication::focusWidget(), static_cast<QWidget*>(&view));
}

void tst_CanvasView::worldPainterUsesExpectedRenderHints()
{
    InputProbeCanvasView view;
    view.resize(320, 240);
    view.show();
    view.viewport()->update();
    QTRY_VERIFY(view.worldDrawn);

    QVERIFY(view.worldHints.testFlag(QPainter::Antialiasing));
    QVERIFY(view.worldHints.testFlag(QPainter::TextAntialiasing));
    QVERIFY(!view.worldHints.testFlag(QPainter::SmoothPixmapTransform));
}

void tst_CanvasView::selectionOutlineUsesShapeColorAndAnimatedDash()
{
    const QColor shapeColor(26, 115, 232);
    const QPen pen = xcanvas::SelectionOutlineStyle::pen(shapeColor, 3.0);

    QCOMPARE(pen.color(), shapeColor);
    QCOMPARE(pen.style(), Qt::CustomDashLine);
    QVERIFY(pen.isCosmetic());
    QCOMPARE(pen.widthF(), 1.0);
    QCOMPARE(pen.capStyle(), Qt::FlatCap);
    QCOMPARE(pen.dashPattern(), QList<qreal>({4.0, 4.0}));
    QCOMPARE(pen.dashOffset(), -3.0);
    QCOMPARE(xcanvas::SelectionOutlineStyle::advancePhase(0.0), 1.0);
    QCOMPARE(xcanvas::SelectionOutlineStyle::advancePhase(7.0), 0.0);
}

QTEST_MAIN(tst_CanvasView)
#include "tst_CanvasView.moc"
