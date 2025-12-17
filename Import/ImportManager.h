#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include "IFileImporter.h"
#include <QVector>
#include <memory>

class ImportManager
{
  public:
    static ImportManager& instance();

    void               registerImporter(std::unique_ptr<IFileImporter> importer);
    QString            buildDialogFilter() const;
    xcanvas::ShapeList importFile(const QString& filePath, const ImportContext& ctx);

  private:
    ImportManager() = default;
    std::vector<std::unique_ptr<IFileImporter>> m_importers;
};

#endif// IMPORTMANAGER_H
