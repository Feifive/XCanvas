#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H

#include "Shape.h"

namespace xcanvas
{
    class Ellipse : public Shape
    {
    public:
        Ellipse();
        ~Ellipse();

        void translate(const QPointF& offset) override;
        void setEllipse(const QPointF& center, double rx, double ry, double rotationDeg = 0.0);
        void setEllipse(const QRectF& rect);
        void setEllipseArc(const QPointF& center, double rx, double ry, double rotationDeg, double startDeg, double endDeg);
        bool isCircle() const;

    protected:
        void updatePainterPath() override;

    private:
        QPointF m_center;
        double  m_rx = 0.0;
        double  m_ry = 0.0;
        double  m_dRotationDegree = 0.0;
        double  m_dStartDegree = 0.0;// arc start
        double  m_dEndDegree = 360.0;// arc end (360 = full ellipse)
    };
}

#endif// ELLIPSESHAPE_H
