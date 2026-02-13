#ifndef XCANVAS_SERIALIZATION_SHAPECODEC_H
#define XCANVAS_SERIALIZATION_SHAPECODEC_H

#include <QJsonObject>

class QString;

namespace xcanvas
{
class Shape;
}

namespace xcanvas::serialization
{
QJsonObject encodeShape(const Shape* shape);
Shape*      decodeShape(const QJsonObject& obj, QString* err = nullptr);
}// namespace xcanvas::serialization

#endif// XCANVAS_SERIALIZATION_SHAPECODEC_H
