#ifndef POLYLINESHAPE_H
#define POLYLINESHAPE_H

#include "Shape.h"
#include <QPointF>
#include <QVector>

namespace xcanvas
{
    class Polyline : public Shape
    {
    public:
        Polyline();
        virtual ~Polyline() override;

        void SetPoints(QVector<QPointF> points);

        void translate(const QPointF& offset) override;

    protected:
        void updatePainterPath() override;

    private:
        QVector<QPointF> m_points;
    };
}



#endif // POLYLINESHAPE_H
