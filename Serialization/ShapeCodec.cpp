#include "ShapeCodec.h"

#include "GeometryDefs.h"
#include "Shape.h"
#include "ShapeImage.h"
#include "ShapeText.h"
#include "ShapeVector.h"

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QJsonArray>
#include <QJsonValue>

namespace xcanvas::serialization
{
namespace
{
constexpr const char* kKeyType      = "type";
constexpr const char* kKeyColor     = "color";
constexpr const char* kKeyVisible   = "visible";
constexpr const char* kKeyLayerId   = "layerId";
constexpr const char* kKeyGroupId   = "groupId";
constexpr const char* kKeyTransform = "transform";

constexpr const char* kTypeVector = "vector";
constexpr const char* kTypeText   = "text";
constexpr const char* kTypeImage  = "image";

constexpr const char* kKeySemantic = "semantic";
constexpr const char* kKeySegments = "segments";
constexpr const char* kKeyPoints   = "points";

constexpr const char* kKeyText = "text";
constexpr const char* kKeyFont = "font";

constexpr const char* kKeyImageData = "imageData";
constexpr const char* kKeySize      = "size";
constexpr const char* kKeyW         = "w";
constexpr const char* kKeyH         = "h";

constexpr const char* kKeyFamily            = "family";
constexpr const char* kKeyPixelSize         = "pixelSize";
constexpr const char* kKeyPointSizeF        = "pointSizeF";
constexpr const char* kKeyWeight            = "weight";
constexpr const char* kKeyItalic            = "italic";
constexpr const char* kKeyUnderline         = "underline";
constexpr const char* kKeyStrikeOut         = "strikeOut";
constexpr const char* kKeyKerning           = "kerning";
constexpr const char* kKeyLetterSpacingType = "letterSpacingType";
constexpr const char* kKeyLetterSpacing     = "letterSpacing";
constexpr const char* kKeyWordSpacing       = "wordSpacing";

QString shapeTypeToString(const ShapeType type)
{
    switch (type)
    {
    case ShapeType::Vector:
        return QString::fromLatin1(kTypeVector);
    case ShapeType::Text:
        return QString::fromLatin1(kTypeText);
    case ShapeType::Image:
        return QString::fromLatin1(kTypeImage);
    }
    return {};
}

QJsonArray encodeTransform(const QTransform& transform)
{
    return {
        transform.m11(),
        transform.m12(),
        transform.m13(),
        transform.m21(),
        transform.m22(),
        transform.m23(),
        transform.m31(),
        transform.m32(),
        transform.m33(),
    };
}

bool decodeTransform(const QJsonValue& value, QTransform* out)
{
    if (!out || !value.isArray())
    {
        return false;
    }

    const QJsonArray arr = value.toArray();
    if (arr.size() != 9)
    {
        return false;
    }

    *out = QTransform(
        arr.at(0).toDouble(1.0), arr.at(1).toDouble(0.0), arr.at(2).toDouble(0.0),
        arr.at(3).toDouble(0.0), arr.at(4).toDouble(1.0), arr.at(5).toDouble(0.0),
        arr.at(6).toDouble(0.0), arr.at(7).toDouble(0.0), arr.at(8).toDouble(1.0));
    return true;
}

QJsonArray encodePointArray(const QPointF& point)
{
    return {point.x(), point.y()};
}

bool decodePointArray(const QJsonValue& value, QPointF* out)
{
    if (!out || !value.isArray())
    {
        return false;
    }
    const QJsonArray arr = value.toArray();
    if (arr.size() != 2)
    {
        return false;
    }
    *out = QPointF(arr.at(0).toDouble(), arr.at(1).toDouble());
    return true;
}

QJsonObject encodeFont(const QFont& font)
{
    QJsonObject obj;
    obj.insert(kKeyFamily, font.family());
    obj.insert(kKeyPixelSize, font.pixelSize());
    obj.insert(kKeyPointSizeF, font.pointSizeF());
    obj.insert(kKeyWeight, font.weight());
    obj.insert(kKeyItalic, font.italic());
    obj.insert(kKeyUnderline, font.underline());
    obj.insert(kKeyStrikeOut, font.strikeOut());
    obj.insert(kKeyKerning, font.kerning());
    obj.insert(kKeyLetterSpacingType, static_cast<int>(font.letterSpacingType()));
    obj.insert(kKeyLetterSpacing, font.letterSpacing());
    obj.insert(kKeyWordSpacing, font.wordSpacing());
    return obj;
}

QFont decodeFont(const QJsonObject& obj)
{
    QFont font;
    if (obj.contains(kKeyFamily))
    {
        font.setFamily(obj.value(kKeyFamily).toString());
    }

    const int pixelSize = obj.value(kKeyPixelSize).toInt(-1);
    if (pixelSize > 0)
    {
        font.setPixelSize(pixelSize);
    }
    else
    {
        const qreal pointSizeF = obj.value(kKeyPointSizeF).toDouble(-1.0);
        if (pointSizeF > 0.0)
        {
            font.setPointSizeF(pointSizeF);
        }
    }

    font.setWeight(static_cast<QFont::Weight>(obj.value(kKeyWeight).toInt(static_cast<int>(font.weight()))));
    font.setItalic(obj.value(kKeyItalic).toBool(font.italic()));
    font.setUnderline(obj.value(kKeyUnderline).toBool(font.underline()));
    font.setStrikeOut(obj.value(kKeyStrikeOut).toBool(font.strikeOut()));
    font.setKerning(obj.value(kKeyKerning).toBool(font.kerning()));
    font.setLetterSpacing(
        static_cast<QFont::SpacingType>(obj.value(kKeyLetterSpacingType).toInt(static_cast<int>(font.letterSpacingType()))),
        obj.value(kKeyLetterSpacing).toDouble(font.letterSpacing()));
    font.setWordSpacing(obj.value(kKeyWordSpacing).toDouble(font.wordSpacing()));
    return font;
}

QJsonArray encodeSegments(const QVector<Segment>& segments)
{
    QJsonArray arr;
    for (const Segment& segment : segments)
    {
        QJsonObject obj;
        obj.insert(kKeyType, static_cast<int>(segment.type));

        QJsonArray points;
        const int  pointCount = segment.pointCount();
        for (int i = 0; i < pointCount; ++i)
        {
            points.append(encodePointArray(segment.point(i)));
        }
        obj.insert(kKeyPoints, points);
        arr.append(obj);
    }
    return arr;
}

bool decodeSegments(const QJsonValue& value, QVector<Segment>* out)
{
    if (!out || !value.isArray())
    {
        return false;
    }

    QVector<Segment> parsed;
    const QJsonArray arr = value.toArray();
    parsed.reserve(arr.size());

    for (const QJsonValue& item : arr)
    {
        if (!item.isObject())
        {
            return false;
        }

        const QJsonObject obj  = item.toObject();
        const int         type = obj.value(kKeyType).toInt(-1);
        if (type < static_cast<int>(SegmentType::MoveTo) || type > static_cast<int>(SegmentType::CubicTo))
        {
            return false;
        }

        Segment segment;
        segment.type = static_cast<SegmentType>(type);
        const int expectedPoints = segment.pointCount();

        const QJsonArray points = obj.value(kKeyPoints).toArray();
        if (points.size() != expectedPoints)
        {
            return false;
        }

        for (int i = 0; i < expectedPoints; ++i)
        {
            QPointF point;
            if (!decodePointArray(points.at(i), &point))
            {
                return false;
            }
            segment.point(i) = point;
        }

        parsed.append(segment);
    }

    *out = std::move(parsed);
    return true;
}

void setCommonShapeFields(const QJsonObject& obj, Shape* shape)
{
    if (!shape)
    {
        return;
    }

    shape->setColor(QColor(obj.value(kKeyColor).toString(shape->color().name(QColor::HexArgb))));
    shape->setVisible(obj.value(kKeyVisible).toBool(true));
    shape->setLayerId(obj.value(kKeyLayerId).toInt(-1));
    shape->setGroupId(obj.value(kKeyGroupId).toString());

    QTransform transform;
    if (decodeTransform(obj.value(kKeyTransform), &transform))
    {
        shape->setTransform(transform);
    }
}

QJsonObject encodeVector(const ShapeVector& shape)
{
    QJsonObject obj;
    obj.insert(kKeySemantic, static_cast<int>(shape.semantic()));
    obj.insert(kKeySegments, encodeSegments(shape.segments()));
    return obj;
}

QJsonObject encodeText(const ShapeText& shape)
{
    QJsonObject obj;
    obj.insert(kKeyText, shape.text());
    obj.insert(kKeyFont, encodeFont(shape.font()));
    return obj;
}

QJsonObject encodeImage(const ShapeImage& shape)
{
    QJsonObject obj;
    QJsonObject size;
    size.insert(kKeyW, shape.imageSize().width());
    size.insert(kKeyH, shape.imageSize().height());
    obj.insert(kKeySize, size);

    QByteArray bytes;
    QBuffer    buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    shape.image().save(&buffer, "PNG");
    obj.insert(kKeyImageData, QString::fromLatin1(bytes.toBase64()));
    return obj;
}
}// namespace

QJsonObject encodeShape(const Shape* shape)
{
    if (!shape)
    {
        return {};
    }

    QJsonObject obj;
    obj.insert(kKeyType, shapeTypeToString(shape->type()));
    obj.insert(kKeyColor, shape->color().name(QColor::HexArgb));
    obj.insert(kKeyVisible, shape->isVisible());
    obj.insert(kKeyLayerId, shape->layerId());
    obj.insert(kKeyGroupId, shape->groupId());
    obj.insert(kKeyTransform, encodeTransform(shape->transform()));

    switch (shape->type())
    {
    case ShapeType::Vector:
        obj.insert(QStringLiteral("data"), encodeVector(*static_cast<const ShapeVector*>(shape)));
        break;
    case ShapeType::Text:
        obj.insert(QStringLiteral("data"), encodeText(*static_cast<const ShapeText*>(shape)));
        break;
    case ShapeType::Image:
        obj.insert(QStringLiteral("data"), encodeImage(*static_cast<const ShapeImage*>(shape)));
        break;
    }
    return obj;
}

Shape* decodeShape(const QJsonObject& obj, QString* err)
{
    const QString type = obj.value(kKeyType).toString();
    const QJsonObject data = obj.value(QStringLiteral("data")).toObject();

    Shape* shape = nullptr;
    if (type == QLatin1String(kTypeVector))
    {
        auto* vector = new ShapeVector();
        vector->setSemantic(static_cast<VectorSemantic>(data.value(kKeySemantic).toInt(static_cast<int>(VectorSemantic::Unknown))));

        QVector<Segment> segments;
        if (!decodeSegments(data.value(kKeySegments), &segments))
        {
            delete vector;
            if (err)
            {
                *err = QStringLiteral("Invalid vector segments");
            }
            return nullptr;
        }
        vector->setSegments(std::move(segments));
        shape = vector;
    }
    else if (type == QLatin1String(kTypeText))
    {
        auto* text = new ShapeText();
        text->setText(data.value(kKeyText).toString());
        text->setFont(decodeFont(data.value(kKeyFont).toObject()));
        shape = text;
    }
    else if (type == QLatin1String(kTypeImage))
    {
        const QString   encoded = data.value(kKeyImageData).toString();
        const QByteArray bytes  = QByteArray::fromBase64(encoded.toLatin1());
        const QImage     image  = QImage::fromData(bytes, "PNG");
        if (image.isNull())
        {
            if (err)
            {
                *err = QStringLiteral("Invalid image data");
            }
            return nullptr;
        }

        auto* imageShape = new ShapeImage(image);
        const QJsonObject sizeObj = data.value(kKeySize).toObject();
        imageShape->setSize(QSizeF(sizeObj.value(kKeyW).toDouble(), sizeObj.value(kKeyH).toDouble()));
        shape = imageShape;
    }
    else
    {
        if (err)
        {
            *err = QStringLiteral("Unknown shape type");
        }
        return nullptr;
    }

    setCommonShapeFields(obj, shape);
    return shape;
}
}// namespace xcanvas::serialization
