#include "DXFTranslator.h"
#include "EllipseShape.h"
#include "Global.h"
#include "PolylineShape.h"
#include "Shapes.h"

DXFTranslator::DXFTranslator() : m_pShapes(nullptr)
{
}

DXFTranslator::~DXFTranslator()
{
}

bool DXFTranslator::Load(const QString& filePath, Shapes* pShapes)
{
    if (!pShapes)
        return false;

    m_pShapes = pShapes;

    dxfRW reader(filePath.toLocal8Bit().toStdString().c_str());

    qDebug() << "[DXF] Start loading:" << filePath;

    if (reader.read(this, false))
    {
        qDebug() << "[DXF] Load success.";
        return true;
    }

    qDebug() << "[DXF] Load failed.";
    return false;
}

void DXFTranslator::addVport(const DRW_Vport& data)
{
    qDebug() << "[DXF] addVport name =" << data.name.c_str();
}

void DXFTranslator::addInsert(const DRW_Insert& data)
{
    qDebug() << "[DXF] addInsert at (" << data.basePoint.x << data.basePoint.y << "), block=" << data.name.c_str();
}

void DXFTranslator::addViewport(const DRW_Viewport& data)
{
    qDebug() << "[DXF] addViewport";
}

void DXFTranslator::linkImage(const DRW_ImageDef* data)
{
    qDebug() << "[DXF] linkImage" << data->name.c_str();
}

void DXFTranslator::addLine(const DRW_Line& data)
{
    qDebug() << "[DXF] addLine (" << data.basePoint.x << data.basePoint.y << ") -> (" << data.secPoint.x << data.secPoint.y << ")";
}

void DXFTranslator::addCircle(const DRW_Circle& data)
{
    qDebug() << "[DXF] addCircle center=(" << data.basePoint.x << data.basePoint.y << ")"
             << " radius=" << data.radious;

    EllipseShape* pShape = new EllipseShape;
    pShape->SetEllipse(QPointF(data.basePoint.x, data.basePoint.y), data.radious, data.radious, 0.0);

    m_pShapes->AddShape(pShape);
}

void DXFTranslator::addLayer(const DRW_Layer& data)
{
    qDebug() << "[DXF] addLayer name =" << data.name.c_str();
}

void DXFTranslator::addArc(const DRW_Arc& data)
{
    qDebug() << "[DXF] addArc center=(" << data.basePoint.x << data.basePoint.y << ")"
             << " radius=" << data.radious << " start=" << data.staangle << " end=" << data.endangle;
}

void DXFTranslator::addEllipse(const DRW_Ellipse& data)
{
    qDebug() << "[DXF] addEllipse";

    QPointF center(data.basePoint.x, data.basePoint.y);
    double  dMajor          = hypot(data.secPoint.x, data.secPoint.y);
    double  dMinor          = dMajor * data.ratio;
    double  dRotationDegree = std::atan2(data.secPoint.y, data.secPoint.x) * 180.0 / M_PI;
    double  dStartDegree    = data.staparam * 180.0 / M_PI;
    double  dEndDegree      = data.endparam * 180.0 / M_PI;

    EllipseShape* shape = new EllipseShape;

    if (qFuzzyCompare(dStartDegree, 0.0) && qFuzzyCompare(dEndDegree, 2 * 180.0))
    {
        shape->SetEllipse(center, dMajor, dMinor, dRotationDegree);
    }
    else
    {
        shape->SetEllipseArc(center, dMajor, dMinor, dRotationDegree, dStartDegree, dEndDegree);
    }

    m_pShapes->AddShape(shape);
}

void DXFTranslator::addText(const DRW_Text& data)
{
    qDebug() << "[DXF] addText \"" << data.text.c_str() << "\" at (" << data.basePoint.x << data.basePoint.y << ")";
}

void DXFTranslator::addMText(const DRW_MText& data)
{
    qDebug() << "[DXF] addMText \"" << data.text.c_str() << "\"";
}

void DXFTranslator::addDimAlign(const DRW_DimAligned* data)
{
    qDebug() << "[DXF] addDimAlign";
}

void DXFTranslator::addDimLinear(const DRW_DimLinear* data)
{
    qDebug() << "[DXF] addDimLinear";
}

void DXFTranslator::addDimRadial(const DRW_DimRadial* data)
{
    qDebug() << "[DXF] addDimRadial";
}

void DXFTranslator::addDimDiametric(const DRW_DimDiametric* data)
{
    qDebug() << "[DXF] addDimDiametric";
}

void DXFTranslator::addDimAngular(const DRW_DimAngular* data)
{
    qDebug() << "[DXF] addDimAngular";
}

void DXFTranslator::addDimAngular3P(const DRW_DimAngular3p* data)
{
    qDebug() << "[DXF] addDimAngular3P";
}

void DXFTranslator::addDimOrdinate(const DRW_DimOrdinate* data)
{
    qDebug() << "[DXF] addDimOrdinate";
}

void DXFTranslator::addLWPolyline(const DRW_LWPolyline& data)
{
    // qDebug() << "[DXF] addLWPolyline vertexCount =" << data.vertlist.size();

    QVector<QPointF> points;
    points.reserve(data.vertlist.size() + 1);

    for (const auto& v : data.vertlist)
    {
        points.append(ConvertDXFPoint(v->x, v->y));
    }

    // DXF flags 0x01 表示 polyline 是闭合的
    bool closed = (data.flags & 0x01u) != 0;

    if (closed)
    {
        // 若首尾点未重复，则手动补上闭合点
        if (!IsEqual(points.first(), points.last()))
        {
            points.append(points.first());
        }

        // qDebug() << "[DXF] Polyline is CLOSED";
    }

    // 以下忽略 bulge、width、elevation、thickness
    // 专注：转换为你的 PolylineShape

    PolylineShape* pShape = new PolylineShape();
    pShape->SetPoints(points);
    pShape->Select(false);// 默认不选中

    if (m_pShapes)
        m_pShapes->AddShape(pShape);
}

void DXFTranslator::addPolyline(const DRW_Polyline& data)
{
    // qDebug() << "[DXF] addPolyline vertexCount =" << data.vertlist.size();

    QVector<QPointF> points;
    for (const auto& v : data.vertlist)
    {
        points.append(ConvertDXFPoint(v->basePoint.x, v->basePoint.y));
    }

    bool closed = (data.flags & 0x01u) != 0;

    if (closed)
    {
        // qDebug() << "[DXF] Polyline is CLOSED";
    }

    if (closed && points.size() > 1 && !IsEqual(points.first(), points.last()))
    {
        points.append(points.first());
    }

    PolylineShape* pLine = new PolylineShape();
    pLine->SetPoints(points);

    if (m_pShapes)
    {
        m_pShapes->AddShape(pLine);
    }
}

void DXFTranslator::addSpline(const DRW_Spline* data)
{
    qDebug() << "[DXF] addSpline controlPoints=" << data->controllist.size();
}

void DXFTranslator::addPoint(const DRW_Point& data)
{
    qDebug() << "[DXF] addPoint (" << data.basePoint.x << data.basePoint.y << ")";
}

void DXFTranslator::addHatch(const DRW_Hatch* data)
{
    qDebug() << "[DXF] addHatch";
}

void DXFTranslator::addBlock(const DRW_Block& data)
{
    qDebug() << "[DXF] addBlock name=" << data.name.c_str();
}

void DXFTranslator::addLType(const DRW_LType& data)
{
    qDebug() << "[DXF] addLType name=" << data.name.c_str();
}

void DXFTranslator::addImage(const DRW_Image* data)
{
    qDebug() << "[DXF] addImage inserted at (" << data->basePoint.x << data->basePoint.y << ")";
}

void DXFTranslator::setBlock(const int handle)
{
    qDebug() << "[DXF] setBlock handle=" << handle;
}

void DXFTranslator::endBlock()
{
    qDebug() << "[DXF] endBlock";
}

void DXFTranslator::writeBlocks()
{
    qDebug() << "[DXF] writeBlocks";
}

void DXFTranslator::writeBlockRecords()
{
    qDebug() << "[DXF] writeBlockRecords";
}

void DXFTranslator::writeEntities()
{
    qDebug() << "[DXF] writeEntities";
}

void DXFTranslator::writeLTypes()
{
    qDebug() << "[DXF] writeLTypes";
}

void DXFTranslator::writeLayers()
{
    qDebug() << "[DXF] writeLayers";
}

void DXFTranslator::writeAppId()
{
    qDebug() << "[DXF] writeAppId";
}

QPointF DXFTranslator::ConvertDXFPoint(double x, double y)
{
    return QPointF(x, -y);
}
