#ifndef RULERINTERACTIONPOLICY_H
#define RULERINTERACTIONPOLICY_H

#include <QtCore/Qt>

namespace xcanvas
{

class RulerInteractionPolicy final
{
  public:
    static bool blocksMove(
        const bool rulerGestureActive,
        const bool pointerInRuler,
        const Qt::MouseButtons buttons)
    {
        return rulerGestureActive || (pointerInRuler && buttons == Qt::NoButton);
    }
};

}// namespace xcanvas

#endif// RULERINTERACTIONPOLICY_H
