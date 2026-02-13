#ifndef XCANVAS_SERIALIZATION_DOCUMENTIO_H
#define XCANVAS_SERIALIZATION_DOCUMENTIO_H

class QString;

namespace xcanvas
{
class Canvas;
}

namespace xcanvas::serialization
{
bool saveDocument(const Canvas* canvas, const QString& filePath, QString* err = nullptr);
bool loadDocument(Canvas* canvas, const QString& filePath, QString* err = nullptr);
}// namespace xcanvas::serialization

#endif// XCANVAS_SERIALIZATION_DOCUMENTIO_H
