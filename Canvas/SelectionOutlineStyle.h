#ifndef SELECTIONOUTLINESTYLE_H
#define SELECTIONOUTLINESTYLE_H

#include <QColor>
#include <QPen>

namespace xcanvas
{

class SelectionOutlineStyle final
{
  public:
    static constexpr qreal DashLength = 4.0;
    static constexpr qreal GapLength = 4.0;
    static constexpr qreal PhaseStep = 1.0;
    static constexpr qreal CycleLength = DashLength + GapLength;

    static QPen pen(const QColor& shapeColor, const qreal phase)
    {
        QPen pen(shapeColor);
        pen.setWidthF(1.0);
        pen.setCosmetic(true);
        pen.setCapStyle(Qt::FlatCap);
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({DashLength, GapLength});
        pen.setDashOffset(-phase);
        return pen;
    }

    static qreal advancePhase(const qreal phase)
    {
        const qreal advanced = phase + PhaseStep;
        return advanced >= CycleLength ? advanced - CycleLength : advanced;
    }
};

}// namespace xcanvas

#endif// SELECTIONOUTLINESTYLE_H
