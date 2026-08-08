#include "Controller/ShapeRenderer.h"
#include "Shape/ShapeImage.h"
#include "Shape/ShapeVector.h"

#include <QImage>
#include <QPainter>
#include <QtTest>

class tst_ShapeRenderer final : public QObject
{
    Q_OBJECT

private slots:
    void vectorUsesPainterStyle();
    void imageUsesShapeTransformAndRestoresPainter();
};

void tst_ShapeRenderer::vectorUsesPainterStyle()
{
    QImage canvas(32, 32, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    xcanvas::ShapeVector shape;
    shape.moveTo(QPointF(4.0, 8.0));
    shape.lineTo(QPointF(24.0, 8.0));

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(Qt::red, 3.0));

    xcanvas::ShapeRenderer renderer;
    renderer.render(&painter, shape);
    painter.end();

    QCOMPARE(canvas.pixelColor(12, 8), QColor(Qt::red));
}

void tst_ShapeRenderer::imageUsesShapeTransformAndRestoresPainter()
{
    QImage source(2, 2, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::blue);

    xcanvas::ShapeImage shape(source);
    shape.setSize(QSizeF(4.0, 4.0));

    QTransform shapeTransform;
    shapeTransform.translate(10.0, 6.0);
    shape.setTransform(shapeTransform);

    QImage canvas(24, 20, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    QPainter   painter(&canvas);
    QTransform initialTransform;
    initialTransform.translate(2.0, 3.0);
    painter.setTransform(initialTransform);

    xcanvas::ShapeRenderer renderer;
    renderer.render(&painter, shape);

    QCOMPARE(painter.transform(), initialTransform);
    painter.end();

    QCOMPARE(canvas.pixelColor(13, 10), QColor(Qt::blue));
    QCOMPARE(canvas.pixelColor(3, 4), QColor(Qt::transparent));
}

QTEST_APPLESS_MAIN(tst_ShapeRenderer)
#include "tst_ShapeRenderer.moc"
