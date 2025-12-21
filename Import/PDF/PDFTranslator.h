#ifndef PDFTRANSLATOR_H
#define PDFTRANSLATOR_H

#include "ShapeManager.h"
#include "fpdfview.h"

class PDFTranslator
{
public:
    PDFTranslator();
    ~PDFTranslator();

    bool               Load(const QString& filePath);
    xcanvas::ShapeList shapeList();

private:
    void parsePage(FPDF_DOCUMENT doc, FPDF_PAGE page);
    void parsePath(FPDF_PAGEOBJECT pathObj);
    void parseText(FPDF_PAGEOBJECT textObj);
    void parseImage(FPDF_DOCUMENT doc, FPDF_PAGE page, FPDF_PAGEOBJECT imageObj);
    void parseForm(FPDF_PAGEOBJECT formObj);
    QPointF ConvertPDFPoint(double x, double y) const;

private:
    xcanvas::ShapeList m_shapeList;
    double m_pageHeightPt;
};

#endif// PDFTRANSLATOR_H
