#include "CanvasCamera.h"
#include "Global.h"

#include <QtTest>

#include <cmath>
#include <limits>

namespace
{
constexpr qreal kTolerance = 1e-9;

void compareReal(const qreal actual, const qreal expected)
{
    QVERIFY2(std::abs(actual - expected) <= kTolerance,
             qPrintable(QStringLiteral("actual=%1 expected=%2").arg(actual, 0, 'g', 16).arg(expected, 0, 'g', 16)));
}

void comparePoint(const QPointF& actual, const QPointF& expected)
{
    compareReal(actual.x(), expected.x());
    compareReal(actual.y(), expected.y());
}

void compareRect(const QRectF& actual, const QRectF& expected)
{
    comparePoint(actual.topLeft(), expected.topLeft());
    compareReal(actual.width(), expected.width());
    compareReal(actual.height(), expected.height());
}
}// namespace

class tst_CanvasCamera final : public QObject
{
    Q_OBJECT

  private slots:
    void defaultsAndBounds();
    void pointAndTransformRoundTrip();
    void rectangleMappingAndVisibleRect();
    void zoomPreservesAnchor();
    void panUsesViewPixels();
    void fitCalculations();
    void rejectsInvalidGeometryWithoutChangingState();
    void repeatedOperationsRemainStable();
};

void tst_CanvasCamera::defaultsAndBounds()
{
    xcanvas::CanvasCamera camera;
    QCOMPARE(camera.scale(), 1.0);
    QCOMPARE(camera.center(), QPointF());

    QVERIFY(camera.setScale(MAX_ZOOM * 2.0));
    QCOMPARE(camera.scale(), static_cast<qreal>(MAX_ZOOM));
    QVERIFY(camera.setScale(MIN_ZOOM / 2.0));
    QCOMPARE(camera.scale(), static_cast<qreal>(MIN_ZOOM));
    QVERIFY(!camera.setScale(0.0));
    QCOMPARE(camera.scale(), static_cast<qreal>(MIN_ZOOM));
}

void tst_CanvasCamera::pointAndTransformRoundTrip()
{
    const xcanvas::CanvasCamera camera(2.5, QPointF(100.0, -40.0));
    const QSizeF viewport(800.0, 600.0);
    const QPointF worldPoint(132.0, 8.0);

    comparePoint(camera.mapFromWorld(camera.center(), viewport), QPointF(400.0, 300.0));
    comparePoint(camera.mapFromWorld(worldPoint, viewport), QPointF(480.0, 420.0));
    comparePoint(camera.mapToWorld(camera.mapFromWorld(worldPoint, viewport), viewport), worldPoint);

    const QTransform identity = camera.viewToWorld(viewport) * camera.worldToView(viewport);
    QVERIFY(identity.isIdentity());
}

void tst_CanvasCamera::rectangleMappingAndVisibleRect()
{
    const xcanvas::CanvasCamera camera(2.0, QPointF(100.0, 50.0));
    const QSizeF viewport(400.0, 200.0);
    const QRectF worldRect(75.0, 25.0, 50.0, 50.0);

    compareRect(camera.mapFromWorld(worldRect, viewport), QRectF(150.0, 50.0, 100.0, 100.0));
    compareRect(camera.mapToWorld(QRectF(150.0, 50.0, 100.0, 100.0), viewport), worldRect);
    compareRect(camera.visibleWorldRect(viewport), QRectF(0.0, 0.0, 200.0, 100.0));
}

void tst_CanvasCamera::zoomPreservesAnchor()
{
    xcanvas::CanvasCamera camera(1.0, QPointF(10.0, 20.0));
    const QSizeF viewport(1000.0, 500.0);
    const QPointF anchor(125.0, 400.0);
    const QPointF before = camera.mapToWorld(anchor, viewport);

    QVERIFY(camera.zoomAt(anchor, 4.0, viewport));
    QCOMPARE(camera.scale(), 4.0);
    comparePoint(camera.mapToWorld(anchor, viewport), before);

    QVERIFY(camera.zoomAt(anchor, MAX_ZOOM * 10.0, viewport));
    QCOMPARE(camera.scale(), static_cast<qreal>(MAX_ZOOM));
    comparePoint(camera.mapToWorld(anchor, viewport), before);
}

void tst_CanvasCamera::panUsesViewPixels()
{
    xcanvas::CanvasCamera camera(2.0, QPointF(100.0, 100.0));
    QVERIFY(camera.panByViewDelta(QPointF(20.0, -10.0)));
    comparePoint(camera.center(), QPointF(90.0, 105.0));
    QVERIFY(!camera.panByViewDelta(QPointF()));
}

void tst_CanvasCamera::fitCalculations()
{
    const QRectF target(100.0, 200.0, 400.0, 100.0);
    const QSizeF viewport(800.0, 600.0);

    const auto width = xcanvas::CanvasCamera::calculateFit(target, viewport, xcanvas::CanvasCamera::FitMode::Width);
    QVERIFY(width.has_value());
    QCOMPARE(width->scale, 2.0);
    QCOMPARE(width->center, QPointF(300.0, 250.0));

    const auto height = xcanvas::CanvasCamera::calculateFit(target, viewport, xcanvas::CanvasCamera::FitMode::Height);
    QVERIFY(height.has_value());
    QCOMPARE(height->scale, 6.0);

    const auto contain = xcanvas::CanvasCamera::calculateFit(target, viewport, xcanvas::CanvasCamera::FitMode::Contain);
    QVERIFY(contain.has_value());
    QCOMPARE(contain->scale, 2.0);

    xcanvas::CanvasCamera camera;
    QVERIFY(camera.fitToRect(target, viewport, xcanvas::CanvasCamera::FitMode::Contain));
    QCOMPARE(camera.scale(), 2.0);
    QCOMPARE(camera.center(), target.center());

    const QRectF huge(0.0, 0.0, 1000000.0, 1000000.0);
    const auto minFit = xcanvas::CanvasCamera::calculateFit(huge, QSizeF(10.0, 10.0), xcanvas::CanvasCamera::FitMode::Contain);
    QVERIFY(minFit.has_value());
    QCOMPARE(minFit->scale, static_cast<qreal>(MIN_ZOOM));
}

void tst_CanvasCamera::rejectsInvalidGeometryWithoutChangingState()
{
    xcanvas::CanvasCamera camera(2.0, QPointF(3.0, 4.0));
    const qreal nan = std::numeric_limits<qreal>::quiet_NaN();

    QVERIFY(!camera.zoomAt(QPointF(10.0, 10.0), 3.0, QSizeF()));
    QVERIFY(!camera.zoomAt(QPointF(nan, 10.0), 3.0, QSizeF(100.0, 100.0)));
    QVERIFY(!camera.panByViewDelta(QPointF(nan, 1.0)));
    QVERIFY(!camera.centerOn(QPointF(1.0, nan)));
    QCOMPARE(camera.scale(), 2.0);
    QCOMPARE(camera.center(), QPointF(3.0, 4.0));

    QVERIFY(!xcanvas::CanvasCamera::calculateFit(QRectF(), QSizeF(100.0, 100.0), xcanvas::CanvasCamera::FitMode::Contain));
    QVERIFY(!xcanvas::CanvasCamera::calculateFit(QRectF(0.0, 0.0, 10.0, 10.0), QSizeF(0.0, 100.0), xcanvas::CanvasCamera::FitMode::Contain));
    QVERIFY(camera.visibleWorldRect(QSizeF()).isEmpty());
}

void tst_CanvasCamera::repeatedOperationsRemainStable()
{
    xcanvas::CanvasCamera camera(1.0, QPointF(1234.5, -987.25));
    const QSizeF viewport(1920.0, 1080.0);
    const QPointF anchor(317.25, 811.75);
    const QPointF originalCenter = camera.center();

    for (int i = 0; i < 500; ++i)
    {
        QVERIFY(camera.zoomAt(anchor, 1.1, viewport));
        QVERIFY(camera.zoomAt(anchor, 1.0, viewport));
        QVERIFY(camera.panByViewDelta(QPointF(0.25, -0.75)));
        QVERIFY(camera.panByViewDelta(QPointF(-0.25, 0.75)));
    }

    comparePoint(camera.center(), originalCenter);
    const QPointF sample(-321.125, 654.875);
    comparePoint(camera.mapToWorld(camera.mapFromWorld(sample, viewport), viewport), sample);
}

QTEST_APPLESS_MAIN(tst_CanvasCamera)

#include "tst_CanvasCamera.moc"
