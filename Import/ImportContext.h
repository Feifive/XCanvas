#ifndef IMPORTCONTEXT_H
#define IMPORTCONTEXT_H

#include <QPointF>

struct ImportContext
{
    QPointF targetCenter;
    double  defaultDpi = 96.0;
    bool    autoCenter = true;
};

#endif// IMPORTCONTEXT_H
