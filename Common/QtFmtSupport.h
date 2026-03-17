#ifndef XCANVAS_QTFMTSUPPORT_H
#define XCANVAS_QTFMTSUPPORT_H

#include <QRectF>
#include <QByteArray>
#include <spdlog/fmt/fmt.h>

template<>
struct fmt::formatter<QString>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QString& s, FormatContext& ctx) const
    {
        QByteArray utf8 = s.toUtf8();
        return fmt::format_to(ctx.out(), "{}", utf8.constData());
    }
};

template<>
struct fmt::formatter<QByteArray>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QByteArray& s, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", fmt::string_view(s.constData(), static_cast<size_t>(s.size())));
    }
};

template<>
struct fmt::formatter<QPointF>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QPointF& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.x(), p.y());
    }
};

template<>
struct fmt::formatter<QRectF>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QRectF& r, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "({}, {}, {}, {})",
            r.x(),
            r.y(),
            r.width(),
            r.height());
    }
};

#endif //XCANVAS_QTFMTSUPPORT_H
