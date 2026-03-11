#ifndef XCANVASICON_H
#define XCANVASICON_H

#include <qtfluentwidgets.h>

enum class XCanvasIconType
{
    MainMenu,
    Import,
    Undo,
    Redo,
    ZoomIn,
    ZoomOut,
    Angle,
    KeepAspectRatio,
    AspectRatioClose,
    AlignLeft,
    HorizontalAlignCenter,
    AlignRight,
    AlignTop,
    VerticalAlignCenter,
    AlignBottom,
    AlignCenter,
    MirrorHorizontally,
    MirrorVertically,
    Union,
    Intersection,
    SubtractA,
    SubtractB,
    Polyline,
    Curve,
    Rect,
    Ellipse,
    Polygon,
    Text,
    Select
};

class XCanvasIcon final : public qfw::FluentIconBase
{
public:
    explicit XCanvasIcon(XCanvasIconType type);

    QString path(qfw::Theme theme = qfw::Theme::Auto) const override;
    void render(QPainter* painter, const QRect& rect, qfw::Theme theme = qfw::Theme::Auto,
                const QVariantMap& attributes = {}) const override;
    qfw::FluentIconBase* clone() const override;

private:
    static QString iconName(XCanvasIconType type);
    static qfw::Theme resolveTheme(qfw::Theme theme);
    QString resolvePath(qfw::Theme theme, bool reverse) const;

private:
    XCanvasIconType m_type;
};

#endif // XCANVASICON_H
