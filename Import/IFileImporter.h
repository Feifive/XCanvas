#ifndef IFILEIMPORTER_H
#define IFILEIMPORTER_H

#include "ImportContext.h"
#include "ShapeManager.h"
#include <QSet>
#include <QString>

enum class ImportCategory
{
    Raster,// 位图
    Vector// 矢量
};

class IFileImporter
{
  public:
    virtual ~IFileImporter()                                                                 = default;
    virtual QSet<QString>      supportedExtensions() const                                   = 0;
    virtual ImportCategory     category() const                                              = 0;
    virtual bool               canImport(const QString& suffix) const                        = 0;
    virtual xcanvas::ShapeList importFile(const QString& filePath, const ImportContext& ctx) = 0;
};

#endif// IFILEIMPORTER_H
