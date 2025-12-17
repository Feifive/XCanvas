#include "ImageDpiUtil.h"

QRectF ImageDpiUtil::imageRectMm(const QImage& image, double defaultDpi)
{
    double dpiX = image.dotsPerMeterX() * 0.0254;
    double dpiY = image.dotsPerMeterY() * 0.0254;

    if (dpiX <= 1e-3)
    {
        dpiX = defaultDpi;
    }
    if (dpiY <= 1e-3)
    {
        dpiY = dpiX;
    }

    const double wMm = image.width() / dpiX * 25.4;
    const double hMm = image.height() / dpiY * 25.4;

    return QRectF(0, 0, wMm, hMm);
}
