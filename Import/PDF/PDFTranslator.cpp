#include "PDFTranslator.h"
#include "ShapeImage.h"
#include "ShapeVector.h"
#include "fpdf_edit.h"
#include <QDebug>
#include <QImage>

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
    qDebug() << "PDF pageCount:" << pageCount;

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
    m_pageHeightPt     = FPDF_GetPageHeight(page);
    const int objCount = FPDFPage_CountObjects(page);

    for (int i = 0; i < objCount; ++i)
    {
        FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
        if (!obj)
        {
            continue;
        }

        switch (int type = FPDFPageObj_GetType(obj))
        {
        case FPDF_PAGEOBJ_PATH:
            parsePath(obj);
            break;
        case FPDF_PAGEOBJ_TEXT:
            parseText(obj);
            break;
        case FPDF_PAGEOBJ_IMAGE:
            parseImage(doc, page, obj, false);
            break;
        case FPDF_PAGEOBJ_FORM:
            parseForm(doc, page, obj);
            break;
        default:
            break;
        }
    }
}

void PDFTranslator::parsePath(FPDF_PAGEOBJECT pathObj)
{
    qDebug() << "parsePath";
    const int segmentCount = FPDFPath_CountSegments(pathObj);
    if (segmentCount <= 0)
    {
        return;
    }

    // 获取变换矩阵
    FS_MATRIX matrix;
    FPDFPageObj_GetMatrix(pathObj, &matrix);

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

        const int type = FPDFPathSegment_GetType(seg);

        float xPt = 0.0f, yPt = 0.0f;
        FPDFPathSegment_GetPoint(seg, &xPt, &yPt);

        double dTransformedX = matrix.a * xPt + matrix.c * yPt + matrix.e;
        double dTransformedY = matrix.b * xPt + matrix.d * yPt + matrix.f;

        const QPointF pt    = ConvertPDFPoint(dTransformedX, dTransformedY);
        const bool    close = FPDFPathSegment_GetClose(seg);

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

void PDFTranslator::parseText(FPDF_PAGEOBJECT textObj)
{
}

void PDFTranslator::parseImage(const FPDF_DOCUMENT doc, const FPDF_PAGE page, const FPDF_PAGEOBJECT imageObj, bool isFormObj)
{
    float left, bottom, right, top;
    if (!FPDFPageObj_GetBounds(imageObj, &left, &bottom, &right, &top))
    {
        return;
    }
    FPDF_BITMAP bitmap = nullptr;
    if (!isFormObj)
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
    qDebug() << "bitmap format:" << format;
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
        qDebug() << "Unknown bitmap format:" << format;
    }

    FPDFBitmap_Destroy(bitmap);

    if (img.isNull())
    {
        return;
    }

    const QPointF topLeftMm = ConvertPDFPoint(left, top);
    const double  widthMm   = (right - left) * PT_TO_MM;
    const double  heightMm  = (top - bottom) * PT_TO_MM;
    const QRectF  rectMm(topLeftMm.x(), topLeftMm.y(), widthMm, heightMm);

    auto* imageShape = new xcanvas::ShapeImage(img);
    imageShape->setRect(rectMm);
    m_shapeList.append(imageShape);
}

void PDFTranslator::parseForm(const FPDF_DOCUMENT doc, const FPDF_PAGE page, FPDF_PAGEOBJECT formObj)
{
    qDebug() << "parseForm";
    m_pageHeightPt         = FPDF_GetPageHeight(page);
    const int formObjCount = FPDFFormObj_CountObjects(formObj);

    for (int i = 0; i < formObjCount; ++i)
    {
        FPDF_PAGEOBJECT obj = FPDFFormObj_GetObject(formObj, i);
        if (!obj)
        {
            continue;
        }

        switch (int type = FPDFPageObj_GetType(obj))
        {
        case FPDF_PAGEOBJ_PATH:
            parsePath(obj);
            break;
        case FPDF_PAGEOBJ_TEXT:
            parseText(obj);
            break;
        case FPDF_PAGEOBJ_IMAGE:
            parseImage(doc, page, obj, true);
            break;
        case FPDF_PAGEOBJ_FORM:
            parseForm(doc, page, obj);
            break;
        default:
            break;
        }
    }
}

QPointF PDFTranslator::ConvertPDFPoint(const double x, const double y) const
{
    const double xMm = x * PT_TO_MM;
    const double yMm = (m_pageHeightPt - y) * PT_TO_MM;
    return {xMm, yMm};
}

QTransform PDFTranslator::pdfMatrixToQt(const FS_MATRIX& m, double pageHeightPt)
{
    QTransform flip(1, 0, 0, -1, 0, pageHeightPt);
    QTransform t(m.a, m.b, m.c, m.d, m.e, m.f);
    return flip * t;
}

QColor PDFTranslator::getFillColor(FPDF_PAGEOBJECT obj, bool& hasFill)
{
    unsigned int r = 0, g = 0, b = 0, a = 255;
    hasFill = FPDFPageObj_GetFillColor(obj, &r, &g, &b, &a);
    return QColor(r, g, b, a);
}

QColor PDFTranslator::getStrokeColor(FPDF_PAGEOBJECT obj, bool& hasStroke)
{
    unsigned int r = 0, g = 0, b = 0, a = 255;
    hasStroke = FPDFPageObj_GetStrokeColor(obj, &r, &g, &b, &a);
    return QColor(r, g, b, a);
}
