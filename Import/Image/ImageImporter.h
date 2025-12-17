#ifndef IMAGEIMPORTER_H
#define IMAGEIMPORTER_H

#include "../IFileImporter.h"

class ImageImporter : public IFileImporter
{
  public:
    QSet<QString>      supportedExtensions() const override;
    ImportCategory     category() const override;
    bool               canImport(const QString& suffix) const override;
    xcanvas::ShapeList importFile(const QString& filePath, const ImportContext& ctx) override;
};

#endif// IMAGEIMPORTER_H
