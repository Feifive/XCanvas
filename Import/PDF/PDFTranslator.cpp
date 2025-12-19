#include "PDFTranslator.h"
#include "fpdfview.h"

#include <QDebug>

PDFTranslator::PDFTranslator()
{
    FPDF_InitLibrary();
}

PDFTranslator::~PDFTranslator()
{
    FPDF_DestroyLibrary();
}

bool PDFTranslator::Load(const QString& filePath)
{

    FPDF_DOCUMENT doc = FPDF_LoadDocument(filePath.toStdString().c_str(), nullptr);
    if (doc)
    {
        int pageCount = FPDF_GetPageCount(doc);
        qDebug() << "PDF pageCount:" << pageCount;
    }

    return true;
}

xcanvas::ShapeList PDFTranslator::shapeList()
{
    return {};
}
