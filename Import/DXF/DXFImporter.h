#ifndef DXFIMPORTER_H
#define DXFIMPORTER_H

#include "../IFileImporter.h"

class DXFImporter : public IFileImporter
{
  public:
    QSet<QString>      supportedExtensions() const override;
    ImportCategory     category() const override;
    bool               canImport(const QString& suffix) const override;
    xcanvas::ShapeList importFile(const QString& filePath, const ImportContext& ctx) override;
};

#endif// DXFIMPORTER_H
