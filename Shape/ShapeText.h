#ifndef TEXT_H
#define TEXT_H

#include "Shape.h"
#include <QFont>

namespace xcanvas
{
class ShapeText final : public Shape
{
  public:
    ShapeText();
    virtual ~ShapeText() override;
    bool hitTest(const QPointF& point, double  tolerance) const override;
    Shape* clone() override;
    ShapeType type() const override;
    void      setFont(const QFont &font);
    QFont     font() const;
    void      setText(const QString& text);
    QString   text() const;
    QPointF   position() const;

  protected:
    void updatePainterPath() override;

  private:
    QFont   m_font;
    QString m_text;
};
}// namespace xcanvas

#endif// TEXT_H
