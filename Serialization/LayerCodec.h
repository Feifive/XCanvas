#ifndef XCANVAS_SERIALIZATION_LAYERCODEC_H
#define XCANVAS_SERIALIZATION_LAYERCODEC_H

#include <QJsonObject>

class QString;

namespace xcanvas
{
struct LayerParameter;
}

namespace xcanvas::serialization
{
QJsonObject encodeLayer(const LayerParameter& layer);
bool        decodeLayer(const QJsonObject& obj, LayerParameter* out, QString* err = nullptr);
}// namespace xcanvas::serialization

#endif// XCANVAS_SERIALIZATION_LAYERCODEC_H
