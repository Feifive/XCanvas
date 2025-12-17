#include "DXFImporter.h"
#include "DXFTranslator.h"

QSet<QString> DXFImporter::supportedExtensions() const
{
    return {"dxf"};
}

ImportCategory DXFImporter::category() const
{
    return ImportCategory::Vector;
}

bool DXFImporter::canImport(const QString& suffix) const
{
    return suffix == "dxf";
}

xcanvas::ShapeList DXFImporter::importFile(const QString& filePath, const ImportContext& ctx)
{
    DXFTranslator translator;
    translator.Load(filePath);
    return translator.shapeList();
}
