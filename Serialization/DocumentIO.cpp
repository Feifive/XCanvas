#include "DocumentIO.h"

#include "DocumentTypes.h"
#include "LayerCodec.h"
#include "ShapeCodec.h"

#include "../Canvas/Canvas.h"
#include "../Layer/LayerManager.h"
#include "../Shape/Shape.h"
#include "../Shape/ShapeManager.h"

#include <QDateTime>
#include <QProcessEnvironment>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QThread>
#include <QUndoStack>

#include "../Common/LogMacros.h"

#include <limits>
#include <zstd.h>

namespace xcanvas::serialization
{
namespace
{
constexpr const char* kKeyX = "x";
constexpr const char* kKeyY = "y";
constexpr const char* kKeyW = "w";
constexpr const char* kKeyH = "h";

int resolveZstdThreads(const QProcessEnvironment& env)
{
    bool envOk = false;
    int  envThreads = env.value(QString::fromLatin1(kZstdThreadsEnvKey)).toInt(&envOk);
    if (envOk)
    {
        return qBound(0, envThreads, 256);
    }

    const int ideal = QThread::idealThreadCount();
    if (ideal <= 1)
    {
        return 0;
    }
    return qBound(2, ideal, 64);
}

QByteArray compressWithZstd(const QByteArray& input, const int level, const int workers, QString* err)
{
    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    if (!cctx)
    {
        if (err)
        {
            *err = QStringLiteral("Failed to create zstd context");
        }
        return {};
    }

    size_t code = ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);
    if (ZSTD_isError(code))
    {
        if (err)
        {
            *err = QStringLiteral("Failed to set zstd level: %1").arg(QString::fromLatin1(ZSTD_getErrorName(code)));
        }
        ZSTD_freeCCtx(cctx);
        return {};
    }

    code = ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, workers);
    if (ZSTD_isError(code))
    {
        if (err)
        {
            *err = QStringLiteral("Failed to set zstd workers: %1").arg(QString::fromLatin1(ZSTD_getErrorName(code)));
        }
        ZSTD_freeCCtx(cctx);
        return {};
    }

    const size_t bound = ZSTD_compressBound(static_cast<size_t>(input.size()));
    QByteArray   output;
    output.resize(static_cast<int>(bound));

    const size_t written = ZSTD_compress2(cctx, output.data(), bound, input.constData(), static_cast<size_t>(input.size()));
    ZSTD_freeCCtx(cctx);
    if (ZSTD_isError(written))
    {
        if (err)
        {
            *err = QStringLiteral("Zstd compress failed: %1").arg(QString::fromLatin1(ZSTD_getErrorName(written)));
        }
        return {};
    }

    output.resize(static_cast<int>(written));
    return output;
}

QByteArray decompressWithZstd(const QByteArray& input, QString* err)
{
    const unsigned long long rawSize = ZSTD_getFrameContentSize(input.constData(), static_cast<size_t>(input.size()));
    if (rawSize == ZSTD_CONTENTSIZE_ERROR)
    {
        if (err)
        {
            *err = QStringLiteral("Invalid zstd frame");
        }
        return {};
    }
    if (rawSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
        if (err)
        {
            *err = QStringLiteral("Unsupported zstd frame with unknown size");
        }
        return {};
    }
    if (rawSize > static_cast<unsigned long long>(std::numeric_limits<int>::max()))
    {
        if (err)
        {
            *err = QStringLiteral("Decompressed payload too large");
        }
        return {};
    }

    QByteArray output;
    output.resize(static_cast<int>(rawSize));

    const size_t decoded = ZSTD_decompress(output.data(), static_cast<size_t>(output.size()), input.constData(), static_cast<size_t>(input.size()));
    if (ZSTD_isError(decoded))
    {
        if (err)
        {
            *err = QStringLiteral("Zstd decompress failed: %1").arg(QString::fromLatin1(ZSTD_getErrorName(decoded)));
        }
        return {};
    }

    output.resize(static_cast<int>(decoded));
    return output;
}

QJsonObject encodeRect(const QRectF& rect)
{
    QJsonObject obj;
    obj.insert(kKeyX, rect.x());
    obj.insert(kKeyY, rect.y());
    obj.insert(kKeyW, rect.width());
    obj.insert(kKeyH, rect.height());
    return obj;
}

QRectF decodeRect(const QJsonObject& obj, const QRectF& fallback = QRectF())
{
    if (!obj.contains(kKeyX) || !obj.contains(kKeyY) || !obj.contains(kKeyW) || !obj.contains(kKeyH))
    {
        return fallback;
    }
    return QRectF(
        obj.value(kKeyX).toDouble(fallback.x()),
        obj.value(kKeyY).toDouble(fallback.y()),
        obj.value(kKeyW).toDouble(fallback.width()),
        obj.value(kKeyH).toDouble(fallback.height()));
}
}// namespace

void clearLoadedDocument(LoadedDocument* doc)
{
    if (!doc)
    {
        return;
    }

    for (Shape* shape : doc->shapes)
    {
        delete shape;
    }
    doc->shapes.clear();
    doc->layers.clear();
    doc->layerOrder.clear();
    doc->canvasRect = QRectF();
}

bool buildDocumentSnapshot(const Canvas* canvas, LoadedDocument* out, QString* err)
{
    if (!out)
    {
        if (err)
        {
            *err = QStringLiteral("Snapshot output is null");
        }
        return false;
    }

    clearLoadedDocument(out);

    if (!canvas)
    {
        if (err)
        {
            *err = QStringLiteral("Canvas is null");
        }
        return false;
    }

    const ShapeManager* shapeManager = canvas->shapeManager();
    const LayerManager* layerManager = canvas->layerManager();
    if (!shapeManager || !layerManager)
    {
        if (err)
        {
            *err = QStringLiteral("Canvas managers are unavailable");
        }
        return false;
    }

    out->canvasRect = canvas->canvasRect();

    out->layerOrder = layerManager->layerOrder();
    out->layers.reserve(out->layerOrder.size());
    for (int id : out->layerOrder)
    {
        const LayerParameter* layer = layerManager->tryGetLayer(id);
        if (!layer)
        {
            continue;
        }

        LayerSnapshot snapshot;
        snapshot.id       = layer->id;
        snapshot.color    = layer->color;
        snapshot.visible  = layer->visible;
        snapshot.output   = layer->output;
        snapshot.mode     = layer->mode;
        snapshot.speed    = layer->speed;
        snapshot.minPower = layer->minPower;
        snapshot.maxPower = layer->maxPower;
        out->layers.append(snapshot);
    }

    const ShapeList sourceShapes = shapeManager->shapes();
    out->shapes.reserve(sourceShapes.size());
    for (const Shape* shape : sourceShapes)
    {
        if (!shape)
        {
            continue;
        }
        if (Shape* copy = const_cast<Shape*>(shape)->clone())
        {
            out->shapes.append(copy);
        }
    }

    return true;
}

bool writeDocument(const LoadedDocument& doc, const QString& filePath, QString* err)
{
    QJsonObject root;
    root.insert(kKeyVersion, kDocumentVersion);
    root.insert(kKeyCanvas, encodeRect(doc.canvasRect));

    QJsonObject meta;
    meta.insert(kKeyApp, QStringLiteral("XCanvas"));
    meta.insert(kKeyTime, QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    root.insert(kKeyMeta, meta);

    QJsonArray layers;
    for (const LayerSnapshot& layer : doc.layers)
    {
        LayerParameter encodedLayer;
        encodedLayer.id       = layer.id;
        encodedLayer.color    = layer.color;
        encodedLayer.visible  = layer.visible;
        encodedLayer.output   = layer.output;
        encodedLayer.mode     = layer.mode;
        encodedLayer.speed    = layer.speed;
        encodedLayer.minPower = layer.minPower;
        encodedLayer.maxPower = layer.maxPower;
        layers.append(encodeLayer(encodedLayer));
    }
    root.insert(kKeyLayers, layers);

    QJsonArray shapes;
    for (const Shape* shape : doc.shapes)
    {
        QJsonObject encoded = encodeShape(shape);
        if (!encoded.isEmpty())
        {
            shapes.append(encoded);
        }
    }
    root.insert(kKeyShapes, shapes);

    QSaveFile out(filePath);
    if (!out.open(QIODevice::WriteOnly))
    {
        if (err)
        {
            *err = QStringLiteral("Failed to open file for writing: %1").arg(filePath);
        }
        return false;
    }

    const QByteArray         jsonPayload = QJsonDocument(root).toJson(QJsonDocument::Indented);
    const QProcessEnvironment env        = QProcessEnvironment::systemEnvironment();
    const bool               savePlain   = env.value(QString::fromLatin1(kPlainSaveEnvKey)) == QStringLiteral("1");

    QByteArray payload;
    if (savePlain)
    {
        payload = jsonPayload;
    }
    else
    {
        bool levelOk = false;
        int  zstdLevel = env.value(QString::fromLatin1(kZstdLevelEnvKey)).toInt(&levelOk);
        if (!levelOk)
        {
            zstdLevel = 6;
        }
        zstdLevel = qBound(1, zstdLevel, 22);

        const int zstdWorkers = resolveZstdThreads(env);
        DOCUMENT_IO_LOG_DEBUG(
            "save zstd: level={} workers={} bytes={}",
            zstdLevel,
            zstdWorkers,
            jsonPayload.size());
        QString    compressErr;
        QByteArray compressed = compressWithZstd(jsonPayload, zstdLevel, zstdWorkers, &compressErr);
        if (compressed.isEmpty() && !jsonPayload.isEmpty())
        {
            if (err)
            {
                *err = compressErr.isEmpty() ? QStringLiteral("Zstd compression failed") : compressErr;
            }
            out.cancelWriting();
            return false;
        }

        payload = QByteArray(kCompressedHeaderZstd);
        payload += compressed;
    }

    if (out.write(payload) != payload.size())
    {
        if (err)
        {
            *err = QStringLiteral("Failed to write complete file: %1").arg(filePath);
        }
        out.cancelWriting();
        return false;
    }

    if (!out.commit())
    {
        if (err)
        {
            *err = QStringLiteral("Failed to commit save file: %1").arg(filePath);
        }
        return false;
    }

    return true;
}

bool readDocument(const QString& filePath, LoadedDocument* out, QString* err)
{
    if (!out)
    {
        if (err)
        {
            *err = QStringLiteral("Loaded document output is null");
        }
        return false;
    }

    clearLoadedDocument(out);

    QFile in(filePath);
    if (!in.open(QIODevice::ReadOnly))
    {
        if (err)
        {
            *err = QStringLiteral("Failed to open file: %1").arg(filePath);
        }
        return false;
    }

    QByteArray fileBytes = in.readAll();
    QByteArray jsonBytes;
    if (fileBytes.startsWith(kCompressedHeaderZstd))
    {
        const QByteArray compressed = fileBytes.mid(static_cast<int>(qstrlen(kCompressedHeaderZstd)));
        QString          decodeErr;
        jsonBytes = decompressWithZstd(compressed, &decodeErr);
        if (jsonBytes.isEmpty() && !compressed.isEmpty())
        {
            if (err)
            {
                *err = decodeErr.isEmpty() ? QStringLiteral("Invalid compressed xcanvas file") : decodeErr;
            }
            return false;
        }
    }
    else
    {
        jsonBytes = fileBytes;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        if (err)
        {
            *err = QStringLiteral("Invalid JSON document: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject root = doc.object();
    const int version = root.value(kKeyVersion).toInt(-1);
    if (version != kDocumentVersion)
    {
        if (err)
        {
            *err = QStringLiteral("Unsupported document version: %1").arg(version);
        }
        return false;
    }

    out->canvasRect = decodeRect(root.value(kKeyCanvas).toObject(), QRectF(10000, 10000, 900, 600));

    const QJsonArray layersArr = root.value(kKeyLayers).toArray();
    const QJsonArray shapesArr = root.value(kKeyShapes).toArray();

    out->layers.reserve(layersArr.size());
    out->layerOrder.reserve(layersArr.size());
    for (const QJsonValue& value : layersArr)
    {
        if (!value.isObject())
        {
            continue;
        }

        LayerParameter layer;
        QString        parseLayerErr;
        if (!decodeLayer(value.toObject(), &layer, &parseLayerErr))
        {
            continue;
        }

        LayerSnapshot snapshot;
        snapshot.id       = layer.id;
        snapshot.color    = layer.color;
        snapshot.visible  = layer.visible;
        snapshot.output   = layer.output;
        snapshot.mode     = layer.mode;
        snapshot.speed    = layer.speed;
        snapshot.minPower = layer.minPower;
        snapshot.maxPower = layer.maxPower;
        out->layers.append(snapshot);
        out->layerOrder.append(snapshot.id);
    }

    out->shapes.reserve(shapesArr.size());
    for (const QJsonValue& value : shapesArr)
    {
        if (!value.isObject())
        {
            continue;
        }
        QString parseShapeErr;
        Shape*  shape = decodeShape(value.toObject(), &parseShapeErr);
        if (shape)
        {
            out->shapes.append(shape);
        }
    }

    return true;
}

bool applyDocumentToCanvas(Canvas* canvas, LoadedDocument&& doc, QString* err)
{
    if (!canvas)
    {
        if (err)
        {
            *err = QStringLiteral("Canvas is null");
        }
        return false;
    }

    ShapeManager* shapeManager = canvas->shapeManager();
    LayerManager* layerManager = canvas->layerManager();
    QUndoStack*   undoStack    = canvas->undoStack();
    if (!shapeManager || !layerManager || !undoStack)
    {
        if (err)
        {
            *err = QStringLiteral("Canvas managers are unavailable");
        }
        return false;
    }

    // Rebuild scene atomically after successful decode to avoid half-loaded state.
    shapeManager->clear();
    layerManager->clearAllLayers();
    undoStack->clear();
    canvas->setCanvasRect(doc.canvasRect);

    QList<int> order;
    order.reserve(doc.layerOrder.size());
    for (const LayerSnapshot& layerSnapshot : doc.layers)
    {
        LayerParameter layer;
        layer.id       = layerSnapshot.id;
        layer.color    = layerSnapshot.color;
        layer.visible  = layerSnapshot.visible;
        layer.output   = layerSnapshot.output;
        layer.mode     = layerSnapshot.mode;
        layer.speed    = layerSnapshot.speed;
        layer.minPower = layerSnapshot.minPower;
        layer.maxPower = layerSnapshot.maxPower;
        layer.shapes.clear();

        if (layerManager->createLayerWithId(layer))
        {
            order.append(layer.id);
        }
    }

    if (!order.isEmpty())
    {
        layerManager->setLayerOrder(order);
    }
    layerManager->rebuildNextId();

    shapeManager->append(doc.shapes);
    for (Shape* shape : doc.shapes)
    {
        if (!shape)
        {
            continue;
        }
        if (!layerManager->bindShapeToLayer(shape, shape->layerId()))
        {
            layerManager->addShapeToLayer(shape);
        }
    }

    shapeManager->deselectAll();
    doc.shapes.clear();
    doc.layers.clear();
    doc.layerOrder.clear();
    return true;
}

bool saveDocument(const Canvas* canvas, const QString& filePath, QString* err)
{
    LoadedDocument snapshot;
    if (!buildDocumentSnapshot(canvas, &snapshot, err))
    {
        clearLoadedDocument(&snapshot);
        return false;
    }

    const bool ok = writeDocument(snapshot, filePath, err);
    clearLoadedDocument(&snapshot);
    return ok;
}

bool loadDocument(Canvas* canvas, const QString& filePath, QString* err)
{
    LoadedDocument loaded;
    if (!readDocument(filePath, &loaded, err))
    {
        clearLoadedDocument(&loaded);
        return false;
    }

    const bool ok = applyDocumentToCanvas(canvas, std::move(loaded), err);
    clearLoadedDocument(&loaded);
    return ok;
}
}// namespace xcanvas::serialization
