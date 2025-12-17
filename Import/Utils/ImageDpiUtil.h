#ifndef IMAGEDPIUTIL_H
#define IMAGEDPIUTIL_H

#include <QImage>
#include <QRectF>

namespace ImageDpiUtil
{
QRectF imageRectMm(const QImage& image, double defaultDpi);
}

#endif// IMAGEDPIUTIL_H
