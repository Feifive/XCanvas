#include "PDFImporter.h"
#include "PDFTranslator.h"

PDFImporter::PDFImporter()
{
}

QSet<QString> PDFImporter::supportedExtensions() const
{
    return {"pdf"};
}

ImportCategory PDFImporter::category() const
{
    return ImportCategory::Vector;
}

bool PDFImporter::canImport(const QString& suffix) const
{
    return suffix == "pdf";
}

xcanvas::ShapeList PDFImporter::importFile(const QString& filePath, const ImportContext& ctx)
{
    PDFTranslator translator;
    translator.Load(filePath);
    return translator.shapeList();
}
