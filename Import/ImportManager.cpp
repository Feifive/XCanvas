#include "ImportManager.h"

#include <QFileInfo>

ImportManager& ImportManager::instance()
{
    static ImportManager importManager;
    return importManager;
}

void ImportManager::registerImporter(std::unique_ptr<IFileImporter> importer)
{
    m_importers.emplace_back(std::move(importer));
}

QString ImportManager::buildDialogFilter() const
{
    QSet<QString> rasterExts;
    QSet<QString> vectorExts;

    for (const auto& importer : m_importers)
    {
        const auto exts = importer->supportedExtensions();
        if (importer->category() == ImportCategory::Raster)
            rasterExts.unite(exts);
        else
            vectorExts.unite(exts);
    }

    auto makeFilter = [](const QString& title, const QSet<QString>& exts)
    {
        QStringList patterns;
        for (const QString& ext : exts)
            patterns << "*." + ext;

        patterns.sort();

        return QString("%1 (%2)").arg(title, patterns.join(' '));
    };

    const QSet<QString> allExts = rasterExts + vectorExts;

    QStringList filters;
    if (!allExts.isEmpty())
        filters << makeFilter(QObject::tr("All Supported Files"), allExts);
    if (!rasterExts.isEmpty())
        filters << makeFilter(QObject::tr("Image Files"), rasterExts);
    if (!vectorExts.isEmpty())
        filters << makeFilter(QObject::tr("Vector Files"), vectorExts);

    filters << QObject::tr("All Files (*)");

    return filters.join(";;");
}

xcanvas::ShapeList ImportManager::importFile(const QString& filePath, const ImportContext& ctx)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();

    for (auto& importer : m_importers)
    {
        if (importer->canImport(suffix))
        {
            return importer->importFile(filePath, ctx);
        }
    }
    return {};
}
