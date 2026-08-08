#include "SelectionUiCoordinator.h"

#include "../Canvas/Canvas.h"
#include "SelectionHudBar.h"
#include "SelectionHudController.h"

SelectionUiCoordinator::SelectionUiCoordinator(
    xcanvas::Canvas* const        canvas,
    SelectionHudBar* const        selectionHudBar,
    SelectionHudController* const selectionHudController,
    UpdateHudBarPos               updateHudBarPos)
    : m_canvas(canvas),
      m_selectionHudBar(selectionHudBar),
      m_selectionHudController(selectionHudController),
      m_updateHudBarPos(std::move(updateHudBarPos))
{
}

void SelectionUiCoordinator::onSelectionChanged()
{
    if (!m_selectionHudBar || !m_canvas || !m_canvas->shapeManager())
    {
        return;
    }

    const auto* shapeManager = m_canvas->shapeManager();
    const bool  hasSelection = shapeManager->hasSelection();
    m_selectionHudBar->setVisible(hasSelection);
    if (hasSelection)
    {
        m_selectionHudBar->setSummary(shapeManager->selectionSummary());
        updateSelectionHud(false);
        if (m_updateHudBarPos)
        {
            m_updateHudBarPos();
        }
    }
}

void SelectionUiCoordinator::updateSelectionHud(const bool isDestroying)
{
    if (m_selectionHudController)
    {
        m_selectionHudController->updateSelectionHud(isDestroying);
    }
}
