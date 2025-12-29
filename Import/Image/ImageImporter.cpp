#include "ImageImporter.h"
#include "../Utils/ImageDpiUtil.h"
#include "ShapeImage.h"
#include <QSet>

QSet<QString> ImageImporter::supportedExtensions() const
{
    return {"png", "jpg", "jpeg", "webp", "bmp", "tif", "tiff", "tga", "gif"};
}

ImportCategory ImageImporter::category() const
{
    return ImportCategory::Raster;
}

bool ImageImporter::canImport(const QString& suffix) const
{
    static const QSet<QString> formats = {"png", "jpg", "jpeg", "webp", "bmp", "tiff", "gif"};
    return formats.contains(suffix);
}

xcanvas::ShapeList ImageImporter::importFile(const QString& filePath, const ImportContext& ctx)
{
    QImage image(filePath);
    if (image.isNull())
    {
        return {};
    }

    QRectF rectMm = ImageDpiUtil::imageRectMm(image, ctx.defaultDpi);

    auto* imageShape = new xcanvas::ShapeImage(image);
    imageShape->setSize(rectMm.size());

    return {imageShape};
}
