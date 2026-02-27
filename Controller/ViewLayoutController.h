#ifndef VIEWLAYOUTCONTROLLER_H
#define VIEWLAYOUTCONTROLLER_H

class BottomFloatingToolBar;
class ColorPaletteWidget;
class QGraphicsView;
class SelectionHudBar;

class ViewLayoutController final
{
  public:
    ViewLayoutController(
        QGraphicsView*         view,
        BottomFloatingToolBar* bottomFloatingToolBar,
        ColorPaletteWidget*    colorPaletteWidget,
        SelectionHudBar*       selectionHudBar);

    void setSelectionHudBar(SelectionHudBar* selectionHudBar);

    void updateBottomFloatingToolBarPos();
    void updateSelectionHudBarPos(bool isDestroying) const;
    void onViewportChanged(bool isDestroying);

  private:
    QGraphicsView*         m_view;
    BottomFloatingToolBar* m_bottomFloatingToolBar;
    ColorPaletteWidget*    m_colorPaletteWidget;
    SelectionHudBar*       m_selectionHudBar;
};

#endif// VIEWLAYOUTCONTROLLER_H
