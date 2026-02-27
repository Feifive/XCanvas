#ifndef SELECTIONUICOORDINATOR_H
#define SELECTIONUICOORDINATOR_H

#include <functional>

class SelectionHudBar;
class SelectionHudController;

namespace xcanvas
{
class Canvas;
}

class SelectionUiCoordinator final
{
  public:
    using UpdateHudBarPos = std::function<void()>;

    SelectionUiCoordinator(
        xcanvas::Canvas*        canvas,
        SelectionHudBar*        selectionHudBar,
        SelectionHudController* selectionHudController,
        UpdateHudBarPos         updateHudBarPos);

    void onSelectionChanged();
    void updateSelectionHud(bool isDestroying);

  private:
    xcanvas::Canvas*        m_canvas;
    SelectionHudBar*        m_selectionHudBar;
    SelectionHudController* m_selectionHudController;
    UpdateHudBarPos         m_updateHudBarPos;
};

#endif// SELECTIONUICOORDINATOR_H
