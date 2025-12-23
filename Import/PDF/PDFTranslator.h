#ifndef PDFTRANSLATOR_H
#define PDFTRANSLATOR_H

#include "ShapeManager.h"
#include "fpdfview.h"
#include <QColor>

class PDFTranslator
{
  public:
    PDFTranslator();
    ~PDFTranslator();

    bool               Load(const QString& filePath);
    xcanvas::ShapeList shapeList();

  private:
    void    parsePage(FPDF_DOCUMENT doc, FPDF_PAGE page);
    void    parsePath(FPDF_PAGEOBJECT pathObj, const QTransform& worldPdfTf);
    void    parseText(FPDF_PAGEOBJECT textObj, const QTransform& worldPdfTf);
    void    parseImage(FPDF_DOCUMENT doc, FPDF_PAGE page, FPDF_PAGEOBJECT imageObj, const QTransform& worldPdfTf, bool insideForm);
    void    parseObjectRecursive(FPDF_DOCUMENT doc, FPDF_PAGE page, FPDF_PAGEOBJECT obj, const QTransform& parentPdfTf, bool insideForm);
    QPointF ConvertPDFPoint(double x, double y) const;

  private:
    QTransform fsMatrixToQTransform(const FS_MATRIX& m);
    bool       isWhiteLike(const QColor& c, int threshold = 245);
    QColor     normalizePdfColor(const QColor& c);
    QColor     getStrokeColor(FPDF_PAGEOBJECT obj, bool& hasStroke);
    QColor     getFillColor(FPDF_PAGEOBJECT obj, bool& hasFill);

  private:
    xcanvas::ShapeList m_shapeList;
    double             m_pageHeightPt;
};

#endif// PDFTRANSLATOR_H
