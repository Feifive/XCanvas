#include "XCanvasIcon.h"

#include <QFile>

XCanvasIcon::XCanvasIcon(const XCanvasIconType type)
    : m_type(type)
{
}

QString XCanvasIcon::path(const qfw::Theme theme) const
{
    return resolvePath(resolveTheme(theme), false);
}

void XCanvasIcon::render(QPainter* const painter, const QRect& rect, const qfw::Theme theme,
                         const QVariantMap& attributes) const
{
    if (!painter)
    {
        return;
    }

    const bool reverse = attributes.value(QStringLiteral("reverse")).toBool();
    const QString iconPath = resolvePath(resolveTheme(theme), reverse);
    if (iconPath.isEmpty())
    {
        return;
    }

    QVariantMap svgAttributes = attributes;
    svgAttributes.remove(QStringLiteral("reverse"));

    if (iconPath.endsWith(QStringLiteral(".svg")))
    {
        const QString svg = qfw::writeSvg(iconPath, QList<int>(), svgAttributes);
        if (!svg.isEmpty())
        {
            qfw::drawSvgIcon(svg.toUtf8(), painter, rect);
            return;
        }
    }

    qfw::drawSvgIcon(iconPath, painter, rect);
}

qfw::FluentIconBase* XCanvasIcon::clone() const
{
    return new XCanvasIcon(*this);
}

QString XCanvasIcon::iconName(const XCanvasIconType type)
{
    switch (type)
    {
        case XCanvasIconType::MainMenu: return QStringLiteral("MainMenu");
        case XCanvasIconType::Import: return QStringLiteral("Import");
        case XCanvasIconType::Undo: return QStringLiteral("Undo");
        case XCanvasIconType::Redo: return QStringLiteral("Redo");
        case XCanvasIconType::ZoomIn: return QStringLiteral("ZoomIn");
        case XCanvasIconType::ZoomOut: return QStringLiteral("ZoomOut");
        case XCanvasIconType::Angle: return QStringLiteral("Angle");
        case XCanvasIconType::KeepAspectRatio: return QStringLiteral("KeepAspectRatio");
        case XCanvasIconType::AspectRatioClose: return QStringLiteral("AspectRatioClose");
        case XCanvasIconType::AlignLeft: return QStringLiteral("AlignLeft");
        case XCanvasIconType::HorizontalAlignCenter: return QStringLiteral("HorizontalAlignCenter");
        case XCanvasIconType::AlignRight: return QStringLiteral("AlignRight");
        case XCanvasIconType::AlignTop: return QStringLiteral("AlignTop");
        case XCanvasIconType::VerticalAlignCenter: return QStringLiteral("VerticalAlignCenter");
        case XCanvasIconType::AlignBottom: return QStringLiteral("AlignBottom");
        case XCanvasIconType::AlignCenter: return QStringLiteral("AlignCenter");
        case XCanvasIconType::MirrorHorizontally: return QStringLiteral("MirrorHorizontally");
        case XCanvasIconType::MirrorVertically: return QStringLiteral("MirrorVertically");
        case XCanvasIconType::Union: return QStringLiteral("Union");
        case XCanvasIconType::Intersection: return QStringLiteral("Intersection");
        case XCanvasIconType::SubtractA: return QStringLiteral("SubtractA");
        case XCanvasIconType::SubtractB: return QStringLiteral("SubtractB");
        case XCanvasIconType::Polyline: return QStringLiteral("Polyline");
        case XCanvasIconType::Curve: return QStringLiteral("Curve");
        case XCanvasIconType::Rect: return QStringLiteral("Rect");
        case XCanvasIconType::Ellipse: return QStringLiteral("Ellipse");
        case XCanvasIconType::Polygon: return QStringLiteral("Polygon");
        case XCanvasIconType::Text: return QStringLiteral("Text");
        case XCanvasIconType::Select: return QStringLiteral("Select");
        case XCanvasIconType::KeepDrawingTool: return QStringLiteral("KeepAspectRatio");
        case XCanvasIconType::DrawingToolLock: return QStringLiteral("DrawingToolLock");
    }
    return QStringLiteral("Select");
}

qfw::Theme XCanvasIcon::resolveTheme(const qfw::Theme theme)
{
    if (theme == qfw::Theme::Auto)
    {
        return qfw::isDarkTheme() ? qfw::Theme::Dark : qfw::Theme::Light;
    }
    return theme;
}

QString XCanvasIcon::resolvePath(const qfw::Theme theme, const bool reverse) const
{
    const QString base = QStringLiteral(":/Resource/Icons/%1").arg(iconName(m_type));
    const QString color = qfw::getIconColor(theme, reverse);
    const QString themedPath = QStringLiteral("%1_%2.svg").arg(base, color);

    if (QFile::exists(themedPath))
    {
        return themedPath;
    }

    const QString fallbackPath = QStringLiteral("%1.svg").arg(base);
    if (QFile::exists(fallbackPath))
    {
        return fallbackPath;
    }

    return QStringLiteral(":/qfluentwidgets/images/icons/Info_%1.svg").arg(color);
}
