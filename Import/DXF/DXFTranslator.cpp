#include "DXFTranslator.h"
#include "Curve.h"
#include "Ellipse.h"
#include "Polyline.h"
#include "libdxfrw.h"

#include <QDebug>

namespace PolylineOptimizer
{

// 精度设置
constexpr double TOLERANCE_DUPLICATE = 0.001;// 去重阈值
constexpr double TOLERANCE_RDP       = 0.01;// RDP 简化阈值

// 计算点到线段的垂直距离
double PerpendicularDistance(const QPointF& p, const QPointF& lineStart, const QPointF& lineEnd)
{
    double dx  = lineEnd.x() - lineStart.x();
    double dy  = lineEnd.y() - lineStart.y();
    double mag = std::hypot(dx, dy);

    // 如果端点重合（或极近），退化为点到点的距离
    if (mag < 1e-9)
    {
        return std::hypot(p.x() - lineStart.x(), p.y() - lineStart.y());
    }

    // 面积法求高: Area / Base
    return std::abs(dy * p.x() - dx * p.y() + lineEnd.x() * lineStart.y() - lineEnd.y() * lineStart.x()) / mag;
}

// RDP 递归逻辑
void RDPRecursive(const QVector<QPointF>& pointList, int start, int end, double epsilon, QVector<bool>& outMask)
{
    if (start + 1 >= end)
        return;

    double maxDist = 0.0;
    int    index   = -1;

    const QPointF& firstPoint = pointList[start];
    const QPointF& lastPoint  = pointList[end];

    for (int i = start + 1; i < end; ++i)
    {
        double dist = PerpendicularDistance(pointList[i], firstPoint, lastPoint);
        if (dist > maxDist)
        {
            maxDist = dist;
            index   = i;
        }
    }

    if (maxDist > epsilon)
    {
        outMask[index] = true;
        RDPRecursive(pointList, start, index, epsilon, outMask);
        RDPRecursive(pointList, index, end, epsilon, outMask);
    }
}

// 核心优化函数
QVector<QPointF> Optimize(const QVector<QPointF>& inputPoints, bool closed)
{
    if (inputPoints.size() < 2)
        return inputPoints;

    // --- 第一步：简单去重 (过滤掉 < 0.001mm 的微小抖动) ---
    QVector<QPointF> noDuplicates;
    noDuplicates.reserve(inputPoints.size());
    noDuplicates.append(inputPoints.first());

    double dupThreshSq = TOLERANCE_DUPLICATE * TOLERANCE_DUPLICATE;
    for (int i = 1; i < inputPoints.size(); ++i)
    {
        double dx = inputPoints[i].x() - noDuplicates.last().x();
        double dy = inputPoints[i].y() - noDuplicates.last().y();
        if (dx * dx + dy * dy > dupThreshSq)
        {
            noDuplicates.append(inputPoints[i]);
        }
    }

    // 处理闭合：如果源数据要求闭合，确保去重后的首尾一致
    if (closed && noDuplicates.size() > 2)
    {
        double dx = noDuplicates.first().x() - noDuplicates.last().x();
        double dy = noDuplicates.first().y() - noDuplicates.last().y();
        // 如果首尾不重合，手动补上
        if (dx * dx + dy * dy > dupThreshSq)
        {
            noDuplicates.append(noDuplicates.first());
        }
        else
        {
            // 如果已经非常接近，强制让最后一个点等于第一个点（为了数学精确）
            noDuplicates.last() = noDuplicates.first();
        }
    }

    if (noDuplicates.size() < 3)
        return noDuplicates;

    // --- 第二步：RDP 算法 (保证 0.01mm 精度) ---
    QVector<bool> keepFlags(noDuplicates.size(), false);
    keepFlags[0]                       = true;
    keepFlags[noDuplicates.size() - 1] = true;

    RDPRecursive(noDuplicates, 0, noDuplicates.size() - 1, TOLERANCE_RDP, keepFlags);

    QVector<QPointF> finalPoints;
    finalPoints.reserve(noDuplicates.size());
    for (int i = 0; i < noDuplicates.size(); ++i)
    {
        if (keepFlags[i])
        {
            finalPoints.append(noDuplicates[i]);
        }
    }

    return finalPoints;
}
}// namespace PolylineOptimizer

// 辅助结构：齐次坐标点 (wx, wy, w)
struct HomogeneousPoint
{
    double x, y, w;

    HomogeneousPoint operator+(const HomogeneousPoint& b) const
    {
        return {x + b.x, y + b.y, w + b.w};
    }
    HomogeneousPoint operator-(const HomogeneousPoint& b) const
    {
        return {x - b.x, y - b.y, w - b.w};
    }
    HomogeneousPoint operator*(double s) const
    {
        return {x * s, y * s, w * s};
    }
};

// 辅助结构：返回欧几里得空间的点和切线
struct SplineGeo
{
    QPointF point;
    QPointF tangent;
};

// 标准 De Boor：返回齐次点 H(t)
static HomogeneousPoint deBoorHomogeneous(double t, int degree, const QVector<double>& knots, const QVector<HomogeneousPoint>& ctrl)
{
    const int n = ctrl.size() - 1;// 控制点索引 0..n
    const int m = knots.size() - 1;// 节点索引 0..m
    const int p = degree;

    if (n < p || m < 2 * p + 1)
        return {0, 0, 1};

    // 找 span: k ∈ [p, m-p-1] 且 t ∈ [U[k], U[k+1])
    int k;
    int kMin = p;
    int kMax = m - p - 1;

    if (t <= knots[kMin])
    {
        k = kMin;
    }
    else if (t >= knots[kMax + 1])
    {
        k = kMax;
    }
    else
    {
        k = kMin;
        for (int i = kMin; i <= kMax; ++i)
        {
            if (t < knots[i + 1])
            {
                k = i;
                break;
            }
        }
    }

    // 初始化 De Boor 点：d[0..p]
    QVector<HomogeneousPoint> d(p + 1);
    for (int j = 0; j <= p; ++j)
    {
        d[j] = ctrl[k - p + j];
    }

    // De Boor 迭代
    for (int r = 1; r <= p; ++r)
    {
        for (int j = p; j >= r; --j)
        {
            int    i     = k - p + j;
            double denom = knots[i + p - r + 1] - knots[i];
            double alpha = 0.0;
            if (denom > 1e-9)
                alpha = (t - knots[i]) / denom;

            d[j] = d[j - 1] * (1.0 - alpha) + d[j] * alpha;
        }
    }

    // 最终点 d[p]
    return d[p];
}

// 只算点：投影齐次坐标 H(t) → 2D 点 P(t)
static QPointF evaluateRationalPoint(double t, int degree, const QVector<double>& knots, const QVector<HomogeneousPoint>& ctrl)
{
    HomogeneousPoint H = deBoorHomogeneous(t, degree, knots, ctrl);
    double           w = (std::abs(H.w) < 1e-12) ? 1.0 : H.w;
    return QPointF(H.x / w, H.y / w);
}

// 点 + 数值切线
static SplineGeo evaluateRationalSplineGeo(double t, int degree, const QVector<double>& knots, const QVector<HomogeneousPoint>& ctrl)
{
    QPointF p = evaluateRationalPoint(t, degree, knots, ctrl);

    // 选一个稳定的 eps（相对整个参数范围）
    double u0    = knots.first();
    double u1    = knots.last();
    double range = u1 - u0;
    if (range < 1e-9)
        return {p, QPointF(0, 0)};

    double eps = range * 1e-4;// 足够小，又不至于被浮点误差吞掉

    double t_minus = std::max(u0, t - eps);
    double t_plus  = std::min(u1, t + eps);

    QPointF p_minus = evaluateRationalPoint(t_minus, degree, knots, ctrl);
    QPointF p_plus  = evaluateRationalPoint(t_plus, degree, knots, ctrl);

    QPointF tangent = (p_plus - p_minus) / (t_plus - t_minus);

    return {p, tangent};
}

DXFTranslator::DXFTranslator()
{
}

DXFTranslator::~DXFTranslator()
{
}

bool DXFTranslator::Load(const QString& filePath)
{
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

xcanvas::ShapeList DXFTranslator::shapeList()
{
    xcanvas::ShapeList result;
    result.swap(m_shapeList);
    return result;
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
    //qDebug() << "[DXF] addLine (" << data.basePoint.x << data.basePoint.y << ") -> (" << data.secPoint.x << data.secPoint.y << ")";

    QVector<QPointF> points;
    points.append(ConvertDXFPoint(data.basePoint.x, data.basePoint.y));
    points.append(ConvertDXFPoint(data.secPoint.x, data.secPoint.y));
    xcanvas::Polyline* pShape = new xcanvas::Polyline;
    pShape->SetPoints(points);

    m_shapeList.append(pShape);
}

void DXFTranslator::addCircle(const DRW_Circle& data)
{
    qDebug() << "[DXF] addCircle center=(" << data.basePoint.x << data.basePoint.y << ")"
             << " radius=" << data.radious;

    xcanvas::Ellipse* pShape = new xcanvas::Ellipse;
    pShape->setEllipse(QPointF(data.basePoint.x, data.basePoint.y), data.radious, data.radious, 0.0);

    m_shapeList.append(pShape);
}

void DXFTranslator::addLayer(const DRW_Layer& data)
{
    qDebug() << "[DXF] addLayer name =" << data.name.c_str();

    QColor layerColor;

    if (data.color24 != -1)
    {
        layerColor = convertTrueColorToQColor(data.color24);
    }
    else
    {
        layerColor = convertAciToQColor(data.color);
    }

    m_layerColors[data.name] = layerColor;
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

    xcanvas::Ellipse* shape = new xcanvas::Ellipse;

    if (qFuzzyCompare(dStartDegree, 0.0) && qFuzzyCompare(dEndDegree, 2 * 180.0))
    {
        shape->setEllipse(center, dMajor, dMinor, dRotationDegree);
    }
    else
    {
        shape->setEllipseArc(center, dMajor, dMinor, dRotationDegree, dStartDegree, dEndDegree);
    }

    m_shapeList.append(shape);
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
    QVector<QPointF> rawPoints;
    rawPoints.reserve(data.vertlist.size());
    for (const auto& v : data.vertlist)
    {
        rawPoints.append(ConvertDXFPoint(v->x, v->y));
    }

    if (rawPoints.isEmpty())
    {
        return;
    }

    bool isClosed = (data.flags & 0x01u) != 0;

    QVector<QPointF> optimizedPoints = PolylineOptimizer::Optimize(rawPoints, isClosed);

    if (optimizedPoints.size() > 1)
    {
        xcanvas::Polyline* pShape = new xcanvas::Polyline;
        pShape->SetPoints(optimizedPoints);
        pShape->setColor(color(data));

        m_shapeList.append(pShape);
    }
}

void DXFTranslator::addPolyline(const DRW_Polyline& data)
{
    // 1. 转换原始数据
    QVector<QPointF> rawPoints;
    rawPoints.reserve(data.vertlist.size());
    for (const auto& v : data.vertlist)
    {
        rawPoints.append(ConvertDXFPoint(v->basePoint.x, v->basePoint.y));
    }

    if (rawPoints.isEmpty())
    {
        return;
    }

    bool isClosed = (data.flags & 0x01u) != 0;

    QVector<QPointF> optimizedPoints = PolylineOptimizer::Optimize(rawPoints, isClosed);

    if (optimizedPoints.size() > 1)
    {
        xcanvas::Polyline* pLine = new xcanvas::Polyline();
        pLine->SetPoints(optimizedPoints);
        pLine->setColor(color(data));

        m_shapeList.append(pLine);
    }
}

void DXFTranslator::addSpline(const DRW_Spline* data)
{
    if (data->controllist.empty() || data->knotslist.empty())
        return;

    if (isHugePseudoSpline(data))
    {
        QVector<QPointF> rawPoints;
        rawPoints.reserve(data->controllist.size());

        for (const auto& c : data->controllist)
        {
            rawPoints.append(ConvertDXFPoint(c->x, c->y));
        }

        bool isClosed = (data->flags & 0x01u) != 0;

        QVector<QPointF> optimizedPoints = PolylineOptimizer::Optimize(rawPoints, isClosed);

        if (optimizedPoints.size() > 1)
        {
            xcanvas::Polyline* poly = new xcanvas::Polyline();
            poly->SetPoints(optimizedPoints);
            poly->setColor(color(*data));
            m_shapeList.append(poly);
        }

        return;
    }

    int degree = data->degree;
    int n      = data->controllist.size();

    // --- 1. 构建齐次坐标控制点 (同前) ---
    QVector<HomogeneousPoint> h_ctrlPoints;
    h_ctrlPoints.reserve(n);

    bool hasWeights = !data->weightlist.empty();
    int  idx        = 0;
    for (const auto& v : data->controllist)
    {
        double w = 1.0;
        if (hasWeights && idx < (int)data->weightlist.size())
        {
            w = data->weightlist[idx];
        }
        QPointF pt2d = ConvertDXFPoint(v->x, v->y);
        h_ctrlPoints.append({pt2d.x() * w, pt2d.y() * w, w});
        idx++;
    }

    QVector<double> knots;
    for (double k : data->knotslist)
        knots.append(k);

    if (knots.size() != h_ctrlPoints.size() + degree + 1)
        return;

    // --- 2. 准备贝塞尔点集 ---
    QVector<QPointF> bezierPoints;
    bool             isFirstPoint = true;

    // --- 3. 核心改进：细分参数 ---
    // 每个 Knot Span 切分成多少段 Bezier？
    // 4 是一个经验值，既能保证圆滑度（即使是90度弧也能完美拟合），又不会产生过多点
    const int SEGMENTS_PER_SPAN = 4;

    for (int i = degree; i < n; ++i)
    {
        double knot_start = knots[i];
        double knot_end   = knots[i + 1];
        double span_total = knot_end - knot_start;

        if (span_total < 1e-9)
            continue;// 跳过空区间

        // 在当前区间内进行细分
        double step = span_total / SEGMENTS_PER_SPAN;

        for (int j = 0; j < SEGMENTS_PER_SPAN; ++j)
        {
            // 计算当前细分段的 t0 和 t1
            double t0 = knot_start + j * step;
            double t1 = knot_start + (j + 1) * step;// 或者是 t0 + step

            // 修正浮点误差，确保最后一段精准落在 knot_end
            if (j == SEGMENTS_PER_SPAN - 1)
                t1 = knot_end;

            double dt = t1 - t0;

            // 采样：计算准确的点和切线
            SplineGeo startGeo = evaluateRationalSplineGeo(t0, degree, knots, h_ctrlPoints);
            SplineGeo endGeo   = evaluateRationalSplineGeo(t1, degree, knots, h_ctrlPoints);

            // Hermite -> Cubic Bezier 转换公式
            // P1 = P0 + V0 * (dt / 3)
            // P2 = P3 - V3 * (dt / 3)
            // 注意：因为我们现在将 dt 变小了，切线长度也会自动缩放，控制点会收缩，
            // 从而把"鼓"出来的方圆部分拉回正确的圆形路径上。

            double factor = dt / 3.0;

            QPointF p0 = startGeo.point;
            QPointF p3 = endGeo.point;
            QPointF p1 = p0 + startGeo.tangent * factor;
            QPointF p2 = p3 - endGeo.tangent * factor;

            // 添加点
            if (isFirstPoint)
            {
                bezierPoints.append(p0);
                isFirstPoint = false;
            }
            bezierPoints.append(p1);
            bezierPoints.append(p2);
            bezierPoints.append(p3);
        }
    }

    // --- 4. 创建 Curve ---
    if (bezierPoints.size() >= 4)
    {
        xcanvas::Curve* pCurve = new xcanvas::Curve();
        pCurve->SetPoints(bezierPoints);
        pCurve->setColor(color(*data));

        m_shapeList.append(pCurve);
    }
}

//void DXFTranslator::addSpline(const DRW_Spline* data)
//{
//    qDebug() << "[DXF] addSpline controlPoints=" << data->controllist.size();
//}

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

QColor DXFTranslator::color(const DRW_Entity& data) const
{
    if (data.color24 != -1)
    {
        return convertTrueColorToQColor(data.color24);
    }
    else if (data.colorName != "")
    {
        // code 430
        int a = 1;
    }
    else
    {
        if (data.color == 256)
        {
            auto it = m_layerColors.find(data.layer);
            if (it != m_layerColors.end())
            {
                return it->second;
            }

            return QColor(0, 0, 0);
        }

        return convertAciToQColor(data.color);
    }

    return QColor(Qt::black);
}

QColor DXFTranslator::convertTrueColorToQColor(int trueColorCode) const
{
    // BGR字节序
    const quint32 color = static_cast<quint32>(trueColorCode);
    const int     red   = color & 0xFF;
    const int     green = (color >> 8) & 0xFF;
    const int     blue  = (color >> 16) & 0xFF;

    return QColor(red, green, blue);
}

QColor DXFTranslator::convertAciToQColor(int aci) const
{
    // 使用静态局部变量存储映射表，确保只初始化一次
    static const std::map<int, QColor> ACI_TO_QCOLOR_MAP = {
        {1, QColor(255, 0, 0)},       {2, QColor(255, 255, 0)},     {3, QColor(0, 255, 0)},       {4, QColor(0, 255, 255)},   {5, QColor(0, 0, 255)},       {6, QColor(255, 0, 255)},
        {7, QColor(0, 0, 0)},         {8, QColor(128, 128, 128)},   {9, QColor(192, 192, 192)},   {10, QColor(255, 0, 0)},    {11, QColor(255, 127, 127)},  {12, QColor(204, 0, 0)},
        {13, QColor(204, 102, 102)},  {14, QColor(153, 0, 0)},      {15, QColor(153, 76, 76)},    {16, QColor(127, 0, 0)},    {17, QColor(127, 63, 63)},    {18, QColor(76, 0, 0)},
        {19, QColor(76, 38, 38)},     {20, QColor(255, 63, 0)},     {21, QColor(255, 159, 127)},  {22, QColor(204, 51, 0)},   {23, QColor(204, 127, 102)},  {24, QColor(153, 38, 0)},
        {25, QColor(153, 95, 76)},    {26, QColor(127, 31, 0)},     {27, QColor(127, 79, 63)},    {28, QColor(76, 19, 0)},    {29, QColor(76, 47, 38)},     {30, QColor(255, 127, 0)},
        {31, QColor(255, 191, 127)},  {32, QColor(204, 102, 0)},    {33, QColor(204, 153, 102)},  {34, QColor(153, 76, 0)},   {35, QColor(153, 114, 76)},   {36, QColor(127, 63, 0)},
        {37, QColor(127, 95, 63)},    {38, QColor(76, 38, 0)},      {39, QColor(76, 57, 38)},     {40, QColor(255, 191, 0)},  {41, QColor(255, 223, 127)},  {42, QColor(204, 153, 0)},
        {43, QColor(204, 178, 102)},  {44, QColor(153, 114, 0)},    {45, QColor(153, 133, 76)},   {46, QColor(127, 95, 0)},   {47, QColor(127, 111, 63)},   {48, QColor(76, 57, 0)},
        {49, QColor(76, 66, 38)},     {50, QColor(255, 255, 0)},    {51, QColor(255, 255, 127)},  {52, QColor(204, 204, 0)},  {53, QColor(204, 204, 102)},  {54, QColor(153, 153, 0)},
        {55, QColor(153, 153, 76)},   {56, QColor(127, 127, 0)},    {57, QColor(127, 127, 63)},   {58, QColor(76, 76, 0)},    {59, QColor(76, 76, 38)},     {60, QColor(191, 255, 0)},
        {61, QColor(223, 255, 127)},  {62, QColor(153, 204, 0)},    {63, QColor(178, 204, 102)},  {64, QColor(114, 153, 0)},  {65, QColor(133, 153, 76)},   {66, QColor(95, 127, 0)},
        {67, QColor(111, 127, 63)},   {68, QColor(57, 76, 0)},      {69, QColor(66, 76, 38)},     {70, QColor(127, 255, 0)},  {71, QColor(191, 255, 127)},  {72, QColor(102, 204, 0)},
        {73, QColor(153, 204, 102)},  {74, QColor(76, 153, 0)},     {75, QColor(114, 153, 76)},   {76, QColor(63, 127, 0)},   {77, QColor(95, 127, 63)},    {78, QColor(38, 76, 0)},
        {79, QColor(57, 76, 38)},     {80, QColor(63, 255, 0)},     {81, QColor(159, 255, 127)},  {82, QColor(51, 204, 0)},   {83, QColor(127, 204, 102)},  {84, QColor(38, 153, 0)},
        {85, QColor(95, 153, 76)},    {86, QColor(31, 127, 0)},     {87, QColor(79, 127, 63)},    {88, QColor(19, 76, 0)},    {89, QColor(47, 76, 38)},     {90, QColor(0, 255, 0)},
        {91, QColor(127, 255, 127)},  {92, QColor(0, 204, 0)},      {93, QColor(102, 204, 102)},  {94, QColor(0, 153, 0)},    {95, QColor(76, 153, 76)},    {96, QColor(0, 127, 0)},
        {97, QColor(63, 127, 63)},    {98, QColor(0, 76, 0)},       {99, QColor(38, 76, 38)},     {100, QColor(0, 255, 63)},  {101, QColor(127, 255, 159)}, {102, QColor(0, 204, 51)},
        {103, QColor(102, 204, 127)}, {104, QColor(0, 153, 38)},    {105, QColor(76, 153, 95)},   {106, QColor(0, 127, 31)},  {107, QColor(63, 127, 79)},   {108, QColor(0, 76, 19)},
        {109, QColor(38, 76, 47)},    {110, QColor(0, 255, 127)},   {111, QColor(127, 255, 191)}, {112, QColor(0, 204, 102)}, {113, QColor(102, 204, 153)}, {114, QColor(0, 153, 76)},
        {115, QColor(76, 153, 114)},  {116, QColor(0, 127, 63)},    {117, QColor(63, 127, 95)},   {118, QColor(0, 76, 38)},   {119, QColor(38, 76, 57)},    {120, QColor(0, 255, 191)},
        {121, QColor(127, 255, 223)}, {122, QColor(0, 204, 153)},   {123, QColor(102, 204, 178)}, {124, QColor(0, 153, 114)}, {125, QColor(76, 153, 133)},  {126, QColor(0, 127, 95)},
        {127, QColor(63, 127, 111)},  {128, QColor(0, 76, 57)},     {129, QColor(38, 76, 66)},    {130, QColor(0, 255, 255)}, {131, QColor(127, 255, 255)}, {132, QColor(0, 204, 204)},
        {133, QColor(102, 204, 204)}, {134, QColor(0, 153, 153)},   {135, QColor(76, 153, 153)},  {136, QColor(0, 127, 127)}, {137, QColor(63, 127, 127)},  {138, QColor(0, 76, 76)},
        {139, QColor(38, 76, 76)},    {140, QColor(0, 191, 255)},   {141, QColor(127, 223, 255)}, {142, QColor(0, 153, 204)}, {143, QColor(102, 178, 204)}, {144, QColor(0, 114, 153)},
        {145, QColor(76, 133, 153)},  {146, QColor(0, 95, 127)},    {147, QColor(63, 111, 127)},  {148, QColor(0, 57, 76)},   {149, QColor(38, 66, 76)},    {150, QColor(0, 127, 255)},
        {151, QColor(127, 191, 255)}, {152, QColor(0, 102, 204)},   {153, QColor(102, 153, 204)}, {154, QColor(0, 76, 153)},  {155, QColor(76, 114, 153)},  {156, QColor(0, 63, 127)},
        {157, QColor(63, 95, 127)},   {158, QColor(0, 38, 76)},     {159, QColor(38, 57, 76)},    {160, QColor(0, 63, 255)},  {161, QColor(127, 159, 255)}, {162, QColor(0, 51, 204)},
        {163, QColor(102, 127, 204)}, {164, QColor(0, 38, 153)},    {165, QColor(76, 95, 153)},   {166, QColor(0, 31, 127)},  {167, QColor(63, 79, 127)},   {168, QColor(0, 19, 76)},
        {169, QColor(38, 47, 76)},    {170, QColor(0, 0, 255)},     {171, QColor(127, 127, 255)}, {172, QColor(0, 0, 204)},   {173, QColor(102, 102, 204)}, {174, QColor(0, 0, 153)},
        {175, QColor(76, 76, 153)},   {176, QColor(0, 0, 127)},     {177, QColor(63, 63, 127)},   {178, QColor(0, 0, 76)},    {179, QColor(38, 38, 76)},    {180, QColor(63, 0, 255)},
        {181, QColor(159, 127, 255)}, {182, QColor(51, 0, 204)},    {183, QColor(127, 102, 204)}, {184, QColor(38, 0, 153)},  {185, QColor(95, 76, 153)},   {186, QColor(31, 0, 127)},
        {187, QColor(79, 63, 127)},   {188, QColor(19, 0, 76)},     {189, QColor(47, 38, 76)},    {190, QColor(127, 0, 255)}, {191, QColor(191, 127, 255)}, {192, QColor(102, 0, 204)},
        {193, QColor(153, 102, 204)}, {194, QColor(76, 0, 153)},    {195, QColor(114, 76, 153)},  {196, QColor(63, 0, 127)},  {197, QColor(95, 63, 127)},   {198, QColor(38, 0, 76)},
        {199, QColor(57, 38, 76)},    {200, QColor(191, 0, 255)},   {201, QColor(223, 127, 255)}, {202, QColor(153, 0, 204)}, {203, QColor(178, 102, 204)}, {204, QColor(114, 0, 153)},
        {205, QColor(133, 76, 153)},  {206, QColor(95, 0, 127)},    {207, QColor(111, 63, 127)},  {208, QColor(57, 0, 76)},   {209, QColor(66, 38, 76)},    {210, QColor(255, 0, 255)},
        {211, QColor(255, 127, 255)}, {212, QColor(204, 0, 204)},   {213, QColor(204, 102, 204)}, {214, QColor(153, 0, 153)}, {215, QColor(153, 76, 153)},  {216, QColor(127, 0, 127)},
        {217, QColor(127, 63, 127)},  {218, QColor(76, 0, 76)},     {219, QColor(76, 38, 76)},    {220, QColor(255, 0, 191)}, {221, QColor(255, 127, 223)}, {222, QColor(204, 0, 153)},
        {223, QColor(204, 102, 178)}, {224, QColor(153, 0, 114)},   {225, QColor(153, 76, 133)},  {226, QColor(127, 0, 95)},  {227, QColor(127, 63, 111)},  {228, QColor(76, 0, 57)},
        {229, QColor(76, 38, 66)},    {230, QColor(255, 0, 127)},   {231, QColor(255, 127, 191)}, {232, QColor(204, 0, 102)}, {233, QColor(204, 102, 153)}, {234, QColor(153, 0, 76)},
        {235, QColor(153, 76, 114)},  {236, QColor(127, 0, 63)},    {237, QColor(127, 63, 95)},   {238, QColor(76, 0, 38)},   {239, QColor(76, 38, 57)},    {240, QColor(255, 0, 63)},
        {241, QColor(255, 127, 159)}, {242, QColor(204, 0, 51)},    {243, QColor(204, 102, 127)}, {244, QColor(153, 0, 38)},  {245, QColor(153, 76, 95)},   {246, QColor(127, 0, 31)},
        {247, QColor(127, 63, 79)},   {248, QColor(76, 0, 19)},     {249, QColor(76, 38, 47)},    {250, QColor(51, 51, 51)},  {251, QColor(91, 91, 91)},    {252, QColor(132, 132, 132)},
        {253, QColor(173, 173, 173)}, {254, QColor(214, 214, 214)}, {255, QColor(255, 255, 255)}};

    // 检查索引是否在有效范围内
    if (aci >= 1 && aci <= 255)
    {
        auto it = ACI_TO_QCOLOR_MAP.find(aci);
        if (it != ACI_TO_QCOLOR_MAP.end())
        {
            return it->second;
        }
    }

    if (aci == 7)
    {
        // 再次明确处理特殊的 7 号索引，确保在默认情况下返回黑色
        return QColor(0, 0, 0);
    }

    // 对于 ByLayer (256)、ByBlock (0) 或无效索引，返回默认颜色 (黑色)
    return QColor(0, 0, 0);
}

bool DXFTranslator::isHugePseudoSpline(const DRW_Spline* s) const
{
    int c = s->controllist.size();
    int k = s->knotslist.size();

    // 经典阈值：超过 2000 点一定是伪样条
    if (c > 2000)
        return true;

    // 控制点 > 500 也强烈建议 fallback
    if (c > 500)
        return true;

    // knots = control + degree + 1 (正常情况)
    // 如果 knots 大量堆叠，也属于伪 Spline
    if (k > c + s->degree + 10)
        return true;

    return false;
}
