#ifndef PDFTRANSLATOR_H
#define PDFTRANSLATOR_H

#include "ShapeManager.h"
#include <QString>

class PDFTranslator
{
  public:
    PDFTranslator();
    ~PDFTranslator();

    bool               Load(const QString& filePath);
    xcanvas::ShapeList shapeList();
};

#endif// PDFTRANSLATOR_H
