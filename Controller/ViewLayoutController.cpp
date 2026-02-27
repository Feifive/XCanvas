#include "ViewLayoutController.h"

#include "../BottomFloatingToolBar.h"
#include "../ColorPaletteWidget.h"
#include "../SelectionHudBar.h"

#include <QGraphicsView>

ViewLayoutController::ViewLayoutController(
    QGraphicsView* const         view,
    BottomFloatingToolBar* const bottomFloatingToolBar,
    ColorPaletteWidget* const    colorPaletteWidget,
    SelectionHudBar* const       selectionHudBar)
    : m_view(view),
      m_bottomFloatingToolBar(bottomFloatingToolBar),
      m_colorPaletteWidget(colorPaletteWidget),
      m_selectionHudBar(selectionHudBar)
{
}

void ViewLayoutController::setSelectionHudBar(SelectionHudBar* const selectionHudBar)
{
    m_selectionHudBar = selectionHudBar;
}

void ViewLayoutController::updateBottomFloatingToolBarPos()
{
    if (!m_view)
    {
        return;
    }

    constexpr int margin = 12;
    QSize         barSize;
    int           x = 0, y = 0;
    if (m_bottomFloatingToolBar)
    {
        barSize = m_bottomFloatingToolBar->sizeHint();
        x       = m_view->width() - barSize.width() - margin;
        y       = m_view->height() - barSize.height() - margin;
        m_bottomFloatingToolBar->move(x, y);
    }

    if (m_colorPaletteWidget)
    {
        barSize = m_colorPaletteWidget->sizeHint();
        int x   = 0;
        int y   = m_view->height() - barSize.height() - margin;
        m_colorPaletteWidget->move(x, y);
    }
}

void ViewLayoutController::updateSelectionHudBarPos(const bool isDestroying) const
{
    if (isDestroying || !m_view || !m_selectionHudBar || !m_selectionHudBar->isVisible())
    {
        return;
    }

    constexpr int marginTop = 12;
    const QSize   barSize   = m_selectionHudBar->sizeHint();
    const int     x         = (m_view->width() - barSize.width()) / 2;
    constexpr int y         = marginTop;

    m_selectionHudBar->move(x, y);
}

void ViewLayoutController::onViewportChanged(const bool isDestroying)
{
    updateBottomFloatingToolBarPos();
    updateSelectionHudBarPos(isDestroying);
}
