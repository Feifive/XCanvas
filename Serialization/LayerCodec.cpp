#include "LayerCodec.h"

#include "../Layer/LayerManager.h"

#include <QColor>

namespace xcanvas::serialization
{
namespace
{
constexpr const char* kKeyId       = "id";
constexpr const char* kKeyColor    = "color";
constexpr const char* kKeyVisible  = "visible";
constexpr const char* kKeyOutput   = "output";
constexpr const char* kKeyMode     = "mode";
constexpr const char* kKeySpeed    = "speed";
constexpr const char* kKeyMinPower = "minPower";
constexpr const char* kKeyMaxPower = "maxPower";
}// namespace

QJsonObject encodeLayer(const LayerParameter& layer)
{
    QJsonObject obj;
    obj.insert(kKeyId, layer.id);
    obj.insert(kKeyColor, layer.color.name(QColor::HexArgb));
    obj.insert(kKeyVisible, layer.visible);
    obj.insert(kKeyOutput, layer.output);
    obj.insert(kKeyMode, static_cast<int>(layer.mode));
    obj.insert(kKeySpeed, layer.speed);
    obj.insert(kKeyMinPower, layer.minPower);
    obj.insert(kKeyMaxPower, layer.maxPower);
    return obj;
}

bool decodeLayer(const QJsonObject& obj, LayerParameter* out, QString* err)
{
    if (!out)
    {
        if (err)
        {
            *err = QStringLiteral("Output layer is null");
        }
        return false;
    }

    if (!obj.contains(kKeyId))
    {
        if (err)
        {
            *err = QStringLiteral("Layer id is missing");
        }
        return false;
    }

    LayerParameter layer;
    layer.id = obj.value(kKeyId).toInt(-1);
    if (layer.id < 0)
    {
        if (err)
        {
            *err = QStringLiteral("Layer id is invalid");
        }
        return false;
    }

    layer.color = QColor(obj.value(kKeyColor).toString(layer.color.name(QColor::HexArgb)));
    if (!layer.color.isValid())
    {
        layer.color = Qt::black;
    }
    layer.visible  = obj.value(kKeyVisible).toBool(true);
    layer.output   = obj.value(kKeyOutput).toBool(true);
    layer.mode     = static_cast<ProcessMode>(obj.value(kKeyMode).toInt(static_cast<int>(ProcessMode::Cut)));
    layer.speed    = obj.value(kKeySpeed).toDouble(100.0);
    layer.minPower = obj.value(kKeyMinPower).toDouble(10.0);
    layer.maxPower = obj.value(kKeyMaxPower).toDouble(20.0);
    layer.shapes.clear();

    *out = std::move(layer);
    return true;
}
}// namespace xcanvas::serialization
