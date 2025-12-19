#ifndef PDFIMPORTER_H
#define PDFIMPORTER_H

#include "../IFileImporter.h"

class PDFImporter : public IFileImporter
{
  public:
    PDFImporter();
    QSet<QString>      supportedExtensions() const override;
    ImportCategory     category() const override;
    bool               canImport(const QString& suffix) const override;
    xcanvas::ShapeList importFile(const QString& filePath, const ImportContext& ctx) override;
};

#endif// PDFIMPORTER_H
