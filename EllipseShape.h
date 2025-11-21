#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H
#include "BaseShape.h"

class EllipseShape : public Shape
{
  public:
    EllipseShape();
    ~EllipseShape();

    void         Offset(const QPointF& offset) override;
    QRectF       boundingRect() const override;
    QPainterPath shape() const override;
    void         SetEllipse(const QPointF& center, double rx, double ry, double rotationDeg = 0.0);
    void         SetEllipseArc(const QPointF& center, double rx, double ry, double rotationDeg, double startDeg, double endDeg);
    bool         IsCircle() const;

  protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void UpdatePath() override;

  private:
    QPointF m_center;
    double  m_rx              = 0.0;
    double  m_ry              = 0.0;
    double  m_dRotationDegree = 0.0;
    double  m_dStartDegree    = 0.0;// arc start
    double  m_dEndDegree      = 360.0;// arc end (360 = full ellipse)
};

#endif// ELLIPSESHAPE_H
