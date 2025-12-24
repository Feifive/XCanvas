#include "PDFTranslator.h"
#include "ShapeImage.h"
#include "ShapeVector.h"
#include "fpdf_edit.h"
#include <QImage>
#include <QtMath>

static constexpr double PT_TO_MM = 25.4 / 72.0;

PDFTranslator::PDFTranslator() : m_pageHeightPt(0.0)
{
}

PDFTranslator::~PDFTranslator()
{
}

bool PDFTranslator::Load(const QString& filePath)
{
    FPDF_InitLibrary();

    FPDF_DOCUMENT doc = FPDF_LoadDocument(filePath.toStdString().c_str(), nullptr);
    if (!doc)
    {
        return false;
    }

    int pageCount = FPDF_GetPageCount(doc);

    for (int i = 0; i < pageCount; ++i)
    {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        if (!page)
        {
            continue;
        }

        parsePage(doc, page);

        FPDF_ClosePage(page);
    }

    FPDF_CloseDocument(doc);
    FPDF_DestroyLibrary();
    return true;
}

xcanvas::ShapeList PDFTranslator::shapeList()
{
    return m_shapeList;
}

void PDFTranslator::parsePage(FPDF_DOCUMENT doc, FPDF_PAGE page)
{
    m_pageHeightPt      = FPDF_GetPageHeight(page);
    const int  objCount = FPDFPage_CountObjects(page);
    QTransform identity;
    for (int i = 0; i < objCount; ++i)
    {
        FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
        if (!obj)
        {
            continue;
        }
        parseObjectRecursive(doc, page, obj, identity, false);
    }
}

void PDFTranslator::parsePath(FPDF_PAGEOBJECT pathObj, const QTransform& worldPdfTf)
{
    const int segmentCount = FPDFPath_CountSegments(pathObj);
    if (segmentCount <= 0)
    {
        return;
    }

    bool   hasStroke   = false;
    bool   hasFill     = false;
    QColor strokeColor = getStrokeColor(pathObj, hasStroke);
    QColor fillColor   = getFillColor(pathObj, hasFill);

    auto* shape = new xcanvas::ShapeVector();
    shape->setSemantic(xcanvas::VectorSemantic::Complex);

    QVector<QPointF> bezierBuf;
    QPointF          currentSubPathStart;
    bool             hasSubPath = false;

    for (int i = 0; i < segmentCount; ++i)
    {
        const FPDF_PATHSEGMENT seg = FPDFPath_GetPathSegment(pathObj, i);
        if (!seg)
        {
            continue;
        }

        float xPt = 0.0f, yPt = 0.0f;
        FPDFPathSegment_GetPoint(seg, &xPt, &yPt);

        QPointF       pPdf  = worldPdfTf.map(QPointF(xPt, yPt));
        const QPointF pt    = ConvertPDFPoint(pPdf.x(), pPdf.y());
        const bool    close = FPDFPathSegment_GetClose(seg);

        const int type = FPDFPathSegment_GetType(seg);
        switch (type)
        {
        case FPDF_SEGMENT_MOVETO:
        {
            // 新子路径开始
            bezierBuf.clear();
            currentSubPathStart = pt;
            hasSubPath          = true;
            shape->moveTo(pt);
            break;
        }

        case FPDF_SEGMENT_LINETO:
        {
            bezierBuf.clear();
            shape->lineTo(pt);
            if (close && hasSubPath)
            {
                if (!qFuzzyCompare(pt, currentSubPathStart))
                {
                    shape->lineTo(currentSubPathStart);
                }
                hasSubPath = false;
            }
            break;
        }

        case FPDF_SEGMENT_BEZIERTO:
        {
            // Bezier 的点要攒 3 个
            bezierBuf.push_back(pt);

            if (bezierBuf.size() == 3)
            {
                shape->cubicTo(bezierBuf[0], bezierBuf[1], bezierBuf[2]);
                QPointF endPt = bezierBuf[2];
                bezierBuf.clear();
                if (close && hasSubPath)
                {
                    if (!qFuzzyCompare(endPt, currentSubPathStart))
                    {
                        shape->lineTo(currentSubPathStart);
                    }
                    hasSubPath = false;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    // Path 有效才加入
    if (!shape->segments().isEmpty())
    {
        if (hasFill && fillColor.red() > 0)
        {
            shape->setColor(fillColor);
        }
        else
        {
            shape->setColor(strokeColor);
        }

        m_shapeList.append(shape);
    }
    else
    {
        delete shape;
    }
}

void PDFTranslator::parseText(FPDF_PAGEOBJECT textObj, const QTransform& worldPdfTf)
{
}

void PDFTranslator::parseImage(const FPDF_DOCUMENT doc, const FPDF_PAGE page, const FPDF_PAGEOBJECT imageObj, const QTransform& worldPdfTf, bool insideForm)
{
    FPDF_BITMAP bitmap = nullptr;
    if (!insideForm)
    {
        bitmap = FPDFImageObj_GetBitmap(imageObj);
    }
    if (!bitmap)
    {
        // 如果原始位图获取失败，再尝试获取渲染位图作为保底
        bitmap = FPDFImageObj_GetRenderedBitmap(doc, page, imageObj);
    }
    if (!bitmap)
    {
        return;
    }

    const int width  = FPDFBitmap_GetWidth(bitmap);
    const int height = FPDFBitmap_GetHeight(bitmap);
    const int format = FPDFBitmap_GetFormat(bitmap);
    const int stride = FPDFBitmap_GetStride(bitmap);
    auto*     buffer = static_cast<unsigned char*>(FPDFBitmap_GetBuffer(bitmap));

    if (!buffer || width <= 0 || height <= 0 || stride <= 0)
    {
        FPDFBitmap_Destroy(bitmap);
        return;
    }

    QImage img;
    if (format == FPDFBitmap_BGR)
    {
        img = QImage(buffer, width, height, stride, QImage::Format_BGR888).copy();
    }
    else if (format == FPDFBitmap_BGRA)
    {
        img = QImage(buffer, width, height, stride, QImage::Format_ARGB32).copy();
    }
    else if (format == FPDFBitmap_Gray)
    {
        img = QImage(buffer, width, height, stride, QImage::Format_Grayscale8).copy();
    }
    else
    {
        // TODO 未处理的图片格式
    }

    FPDFBitmap_Destroy(bitmap);

    if (img.isNull())
    {
        return;
    }

    // 从矩阵提取旋转、位置和大小
    // 获取 PDF 单元矩形的四个角在 mm 空间中的坐标
    // PDF 图片定义在 (0,0) 到 (1,1) 的单位矩形中，通过 worldPdfTf 变换到页面位置
    QPointF p0 = worldPdfTf.map(QPointF(0, 0));// 左下 (PDF 空间)
    QPointF p1 = worldPdfTf.map(QPointF(1, 0));// 右下
    QPointF p2 = worldPdfTf.map(QPointF(0, 1));// 左上

    // 将 PDF 坐标点转换为Qt的 mm 坐标点
    QPointF p0Mm = ConvertPDFPoint(p0.x(), p0.y());
    QPointF p1Mm = ConvertPDFPoint(p1.x(), p1.y());
    QPointF p2Mm = ConvertPDFPoint(p2.x(), p2.y());

    // 计算旋转角度 (利用向量 p0Mm -> p1Mm)
    // 由于 PDF 和 Qt 的 Y 轴方向相反，这里使用 atan2 计算
    double angleRad = std::atan2(p1Mm.y() - p0Mm.y(), p1Mm.x() - p0Mm.x());
    double angleDeg = qRadiansToDegrees(angleRad);

    // 计算在 mm 空间下的真实宽度和高度 (向量长度)
    double widthMm  = std::sqrt(std::pow(p1Mm.x() - p0Mm.x(), 2) + std::pow(p1Mm.y() - p0Mm.y(), 2));
    double heightMm = std::sqrt(std::pow(p2Mm.x() - p0Mm.x(), 2) + std::pow(p2Mm.y() - p0Mm.y(), 2));

    // 计算中心点
    // 单元矩形的中心是 (0.5, 0.5)
    QPointF pdfCenter = worldPdfTf.map(QPointF(0.5, 0.5));
    QPointF centerMm  = ConvertPDFPoint(pdfCenter.x(), pdfCenter.y());

    // 未旋转前的矩形（以中心点对齐）
    QRectF rectMm(centerMm.x() - widthMm / 2.0, centerMm.y() - heightMm / 2.0, widthMm, heightMm);

    auto* imageShape = new xcanvas::ShapeImage(img);
    imageShape->setRect(rectMm);
    imageShape->rotate(angleDeg, QPointF(0, 0));

    m_shapeList.append(imageShape);
}

void PDFTranslator::parseObjectRecursive(FPDF_DOCUMENT doc, FPDF_PAGE page, FPDF_PAGEOBJECT obj, const QTransform& parentPdfTf, bool insideForm)
{
    if (!obj)
    {
        return;
    }

    FS_MATRIX matrix;
    FPDFPageObj_GetMatrix(obj, &matrix);
    QTransform localPdfTf = fsMatrixToQTransform(matrix);
    QTransform worldPdfTf = localPdfTf * parentPdfTf;

    const int type = FPDFPageObj_GetType(obj);
    switch (type)
    {
    case FPDF_PAGEOBJ_PATH:
        parsePath(obj, worldPdfTf);
        break;
    case FPDF_PAGEOBJ_TEXT:
        parseText(obj, worldPdfTf);
        break;
    case FPDF_PAGEOBJ_IMAGE:
        parseImage(doc, page, obj, worldPdfTf, insideForm);
        break;
    case FPDF_PAGEOBJ_FORM:
    {
        const int count = FPDFFormObj_CountObjects(obj);
        for (int i = 0; i < count; ++i)
        {
            FPDF_PAGEOBJECT child = FPDFFormObj_GetObject(obj, i);
            if (!child)
            {
                continue;
            }
            parseObjectRecursive(doc, page, child, worldPdfTf, true);
        }
        break;
    }
    default:
        break;
    }
}

QPointF PDFTranslator::ConvertPDFPoint(const double x, const double y) const
{
    const double xMm = x * PT_TO_MM;
    const double yMm = (m_pageHeightPt - y) * PT_TO_MM;
    return {xMm, yMm};
}

QTransform PDFTranslator::fsMatrixToQTransform(const FS_MATRIX& m)
{
    return QTransform(m.a, m.b, m.c, m.d, m.e, m.f);
}

bool PDFTranslator::isWhiteLike(const QColor& c, int threshold)
{
    return c.alpha() > 0 && c.red() >= threshold && c.green() >= threshold && c.blue() >= threshold;
}

QColor PDFTranslator::normalizePdfColor(const QColor& c)
{
    if (!c.isValid())
    {
        return QColor(Qt::black);
    }
    if (isWhiteLike(c))
    {
        return QColor(0, 0, 0, c.alpha());
    }
    return c;
}

QColor PDFTranslator::getFillColor(FPDF_PAGEOBJECT obj, bool& hasFill)
{
    unsigned int r = 0, g = 0, b = 0, a = 255;
    hasFill = FPDFPageObj_GetFillColor(obj, &r, &g, &b, &a);
    QColor c(r, g, b, a);
    return normalizePdfColor(c);
}

QColor PDFTranslator::getStrokeColor(FPDF_PAGEOBJECT obj, bool& hasStroke)
{
    unsigned int r = 0, g = 0, b = 0, a = 255;
    hasStroke = FPDFPageObj_GetStrokeColor(obj, &r, &g, &b, &a);
    QColor c(r, g, b, a);
    return normalizePdfColor(c);
}
