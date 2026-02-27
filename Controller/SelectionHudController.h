#ifndef SELECTIONHUDCONTROLLER_H
#define SELECTIONHUDCONTROLLER_H

#include <functional>

class SelectionHudBar;

namespace xcanvas
{
class Canvas;
}

class SelectionHudController final
{
  public:
    using RequestFullUpdate = std::function<void()>;

    SelectionHudController(xcanvas::Canvas* canvas, SelectionHudBar* selectionHudBar, RequestFullUpdate requestFullUpdate);

    void setKeepAspectRatio(bool enabled);
    void applySelectionHudX(double newX);
    void applySelectionHudY(double newY);
    void applySelectionHudW(double newW);
    void applySelectionHudH(double newH);
    void applySelectionHudAngle(double newAngle);
    void updateSelectionHud(bool isDestroying);

  private:
    xcanvas::Canvas*  m_canvas;
    SelectionHudBar*  m_selectionHudBar;
    RequestFullUpdate m_requestFullUpdate;
    bool              m_keepAspectRatio;
};

#endif// SELECTIONHUDCONTROLLER_H
