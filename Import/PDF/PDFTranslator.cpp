#include "PDFTranslator.h"
#include "fpdf_edit.h"
#include "fpdf_transformpage.h"
#include "Polyline.h"
#include "Image.h"
#include "Utils/ImageDpiUtil.h"
#include "Vector.h"
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
    if (!doc) {
        return false;
    }

    int pageCount = FPDF_GetPageCount(doc);
    qDebug() << "PDF pageCount:" << pageCount;

    for (int i = 0; i < pageCount; ++i)
    {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        if (!page) {
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

void PDFTranslator::parsePage(FPDF_DOCUMENT doc, FPDF_PAGE page) {
    float left, bottom, right, top;
    FPDFPage_GetMediaBox(page, &left, &bottom, &right, &top);
    m_pageHeightPt = top - bottom;

    const int objCount = FPDFPage_CountObjects(page);

    for (int i = 0; i < objCount; ++i)
    {
        FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
        if (!obj) {
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
                parseImage(doc, page, obj);
                break;
            case FPDF_PAGEOBJ_FORM:
                parseForm(obj);
                break;
            default:
                break;
        }
    }
}

void PDFTranslator::parsePath(FPDF_PAGEOBJECT pathObj)
{
    const int segmentCount = FPDFPath_CountSegments(pathObj);
    if (segmentCount <= 0) {
        return;
    }

    auto* shape = new xcanvas::Vector();
    shape->setSemantic(xcanvas::VectorSemantic::Complex);

    QVector<QPointF> bezierBuf;
    QPointF currentSubPathStart;
    bool hasSubPath = false;

    for (int i = 0; i < segmentCount; ++i)
    {
        const FPDF_PATHSEGMENT seg = FPDFPath_GetPathSegment(pathObj, i);
        if (!seg) {
            continue;
        }

        const int type = FPDFPathSegment_GetType(seg);

        float xPt = 0.0f, yPt = 0.0f;
        FPDFPathSegment_GetPoint(seg, &xPt, &yPt);

        const QPointF pt = ConvertPDFPoint(xPt, yPt);
        const bool close = FPDFPathSegment_GetClose(seg);

        switch (type)
        {
            case FPDF_SEGMENT_MOVETO:
            {
                // 新子路径开始
                bezierBuf.clear();
                currentSubPathStart = pt;
                hasSubPath = true;
                shape->moveTo(pt);
                break;
            }

            case FPDF_SEGMENT_LINETO:
            {
                bezierBuf.clear();
                shape->lineTo(pt);
                if (close && hasSubPath) {
                    if (!qFuzzyCompare(pt, currentSubPathStart)) {
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
                    shape->cubicTo(
                        bezierBuf[0],
                        bezierBuf[1],
                        bezierBuf[2]);
                    QPointF endPt = bezierBuf[2];
                    bezierBuf.clear();
                    if (close && hasSubPath) {
                        if (!qFuzzyCompare(endPt, currentSubPathStart)) {
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
        m_shapeList.append(shape);
    }
    else
    {
        delete shape;
    }
}

void PDFTranslator::parseText(FPDF_PAGEOBJECT textObj) {
}

void PDFTranslator::parseImage(const FPDF_DOCUMENT doc, const FPDF_PAGE page, const FPDF_PAGEOBJECT imageObj) {
    float left, bottom, right, top;
    if (!FPDFPageObj_GetBounds(imageObj, &left, &bottom, &right, &top)) {
        return;
    }
    FPDF_BITMAP bitmap = nullptr;
    bitmap = FPDFImageObj_GetBitmap(imageObj);
    if (!bitmap) {
        // 如果原始位图获取失败，再尝试获取渲染位图作为保底
        bitmap = FPDFImageObj_GetRenderedBitmap(doc, page, imageObj);
    }
    if (!bitmap) {
        return;
    }

    const int width  = FPDFBitmap_GetWidth(bitmap);
    const int height = FPDFBitmap_GetHeight(bitmap);
    const int format = FPDFBitmap_GetFormat(bitmap);
    const int stride = FPDFBitmap_GetStride(bitmap);
    auto* buffer     = static_cast<unsigned char*>(FPDFBitmap_GetBuffer(bitmap));

    if (!buffer || width <= 0 || height <= 0 || stride <= 0) {
        FPDFBitmap_Destroy(bitmap);
        return;
    }

    QImage img;
    if (format == FPDFBitmap_BGR) {
        img = QImage(buffer, width, height, stride, QImage::Format_BGR888).copy();
    }
    else if (format == FPDFBitmap_BGRA) {
        img = QImage(buffer, width, height, stride, QImage::Format_ARGB32_Premultiplied).copy();
    }
    else if (format == FPDFBitmap_Gray) {
        img = QImage(buffer, width, height, stride, QImage::Format_Grayscale8).copy();
    }
    else {
        qDebug() << "Unknown bitmap format:" << format;
    }

    FPDFBitmap_Destroy(bitmap);

    if (img.isNull()) {
        return;
    }

    const QPointF topLeftMm = ConvertPDFPoint(left, top);
    const double widthMm = (right - left) * PT_TO_MM;
    const double heightMm = (top - bottom) * PT_TO_MM;
    const QRectF rectMm(topLeftMm.x(), topLeftMm.y(), widthMm, heightMm);

    auto* imageShape = new xcanvas::Image(img);
    imageShape->setRect(rectMm);
    m_shapeList.append(imageShape);
}

void PDFTranslator::parseForm(FPDF_PAGEOBJECT formObj) {
}

QPointF PDFTranslator::ConvertPDFPoint(const double x, const double y) const {
    const double xMm = x * PT_TO_MM;
    const double yMm = (m_pageHeightPt - y) * PT_TO_MM;
    return {xMm, yMm};
}
