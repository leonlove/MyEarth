#include <deuMath.h>

namespace cmm { namespace math{

double Point2LineDistance(const Point2d &ptTest, const Point2d &pointA, const Point2d &pointB, bool bSegment)
{
    // 这里使用向量几何来解决问题

    const Point2d vec_AB = pointB - pointA;
    const Point2d vec_AT = ptTest - pointA;

    const double fltDot1 = vec_AB * vec_AT;
    if(bSegment && fltDot1 <= 0.0f)
    {
        return vec_AT.length();                // 测试点位于线段左端点还靠左
    }

    const double fltDot2 = vec_AB * vec_AB;
    if(bSegment && fltDot1 >= fltDot2)
    {
        return (ptTest - pointB).length();    // 测试点位于线段右端点还靠右
    }

    const double fltRatio = fltDot1 / fltDot2;
    const Point2d ptProj = pointA + vec_AB * fltRatio;    // 测试点在直线上的投影点

    return (ptProj - ptTest).length();
}

Polygon2::Polygon2(void)
{
}

Polygon2::Polygon2(const Polygon2 &val)
{
    m_vecVertices.assign(val.m_vecVertices.begin(), val.m_vecVertices.end());
}

Polygon2::Polygon2(const std::vector<Point2d> &vertexs)
{
    setVertexs(vertexs);
}

Polygon2::~Polygon2(void)
{
}

void Polygon2::setVertexs(const std::vector<Point2d> &vertices)
{
    m_vecVertices.assign(vertices.begin(), vertices.end());
}


const Polygon2 &Polygon2::operator=(const Polygon2 &val)
{
    if(this == &val)    return *this;
    m_vecVertices = val.m_vecVertices;
    return *this;
}


bool Polygon2::containsPoint(const Point2d &ptTest) const
{
    if(m_vecVertices.size() <= 2)    return false;

    std::vector<Point2d>::const_iterator itorVertex0 = m_vecVertices.end() - 1;

    /* get test bit for above/below X axis */
    bool bFlagY0 = (itorVertex0->y() >= ptTest.y());

    bool bInsideFlag = false;
    std::vector<Point2d>::const_iterator itorVertex1 = m_vecVertices.begin();
    for(; itorVertex1 != m_vecVertices.end(); ++itorVertex1)
    {
        const Point2d &point0 = *itorVertex0;
        const Point2d &point1 = *itorVertex1;
        const bool bFlagY1 = (point1.y() >= ptTest.y());

        /* check if endpoints straddle (are on opposite sides) of X axis
        * (i.e. the Y's differ); if so, +X ray could intersect this edge.
        */
        if(bFlagY0 != bFlagY1)
        {
            const bool bFlagX0 = (point0.x() >= ptTest.x());

            /* check if endpoints are on same side of the Y axis (i.e. X's
            * are the same); if so, it's easy to test if edge hits or misses.
            */
            if (bFlagX0 == (point1.x() >= ptTest.x()))
            {
                /* if edge's X values both right of the ptTest, must hit */
                if (bFlagX0) bInsideFlag = !bInsideFlag;
            }
            else
            {
                /* compute intersection of pgon segment with +X ray, note
                * if >= ptTest's X; if so, the ray hits it.
                */
                if ((point1.x() - (point1.y() - ptTest.y()) *
                    (point0.x() - point1.x()) / (point0.y() - point1.y())) >= ptTest.x())
                {
                    bInsideFlag = !bInsideFlag;
                }
            }
        }

        /* move to next pair of vertices, retaining info as possible */
        bFlagY0 = bFlagY1;
        itorVertex0 = itorVertex1;
    }

    return bInsideFlag;
}


double Polygon2::pointDistance(const Point2d &ptTest) const
{
    double fltDistance = DBL_MAX;//FLT_MAX;
    std::vector<Point2d>::const_iterator itorVertex0 = m_vecVertices.end() - 1;
    std::vector<Point2d>::const_iterator itorVertex1 = m_vecVertices.begin();
    for(; itorVertex1 != m_vecVertices.end(); ++itorVertex1)
    {
        const Point2d &point0 = *itorVertex0;
        const Point2d &point1 = *itorVertex1;
        const double flt = Point2LineDistance(ptTest, point0, point1, true);
        if(flt < fltDistance)
        {
            fltDistance = flt;
        }
        itorVertex0 = itorVertex1;
    }
    return fltDistance;
}


double Polygon2::findNearestSegment(const Point2d &ptTest, Point2d &vtx0, Point2d &vtx1) const
{
    double fltDistance = DBL_MAX;//FLT_MAX;
    std::vector<Point2d>::const_iterator itorVertex0 = m_vecVertices.end() - 1;
    std::vector<Point2d>::const_iterator itorVertex1 = m_vecVertices.begin();
    for(; itorVertex1 != m_vecVertices.end(); ++itorVertex1)
    {
        const Point2d &point0 = *itorVertex0;
        const Point2d &point1 = *itorVertex1;
        const double flt = Point2LineDistance(ptTest, point0, point1, true);
        if(flt < fltDistance)
        {
            vtx0 = point0;
            vtx1 = point1;
            fltDistance = flt;
        }
        itorVertex0 = itorVertex1;
    }
    return fltDistance;
}


double Polygon2::area(void) const
{
    if(m_vecVertices.size() <= 2u)    return 0.0;

    double area = 0.0;
    std::vector<Point2d>::const_iterator itorVertex0 = m_vecVertices.end() - 1u;
    std::vector<Point2d>::const_iterator itorVertex1 = m_vecVertices.begin();
    while(itorVertex1 != m_vecVertices.end())
    {
        area += itorVertex0->x() * itorVertex1->y() - itorVertex1->x() * itorVertex0->y();

        itorVertex0 = itorVertex1;
        ++itorVertex1;
    }
    return area * 0.5;
}


Point2d Polygon2::centroid(void) const
{
    Point2d point(0.0, 0.0);
    if(m_vecVertices.size() <= 2)
    {
        if(m_vecVertices.size() <= 0)    return point;
        if(m_vecVertices.size() == 1)    return m_vecVertices.at(0);

        point += m_vecVertices.at(0);
        point += m_vecVertices.at(1);
        return point * 0.5;
    }

    double area = 0.0;
    std::vector<Point2d>::const_iterator itorVertex0 = m_vecVertices.end() - 1;
    std::vector<Point2d>::const_iterator itorVertex1 = m_vecVertices.begin();
    while(itorVertex1 != m_vecVertices.end())
    {
        const double dT = itorVertex0->x() * itorVertex1->y() - itorVertex1->x() * itorVertex0->y();
        area += dT;

        point.x() += (itorVertex0->x() + itorVertex1->x()) * dT;
        point.y() += (itorVertex0->y() + itorVertex1->y()) * dT;

        itorVertex0 = itorVertex1;
        ++itorVertex1;
    }

    const double dN = 3.0 * area;
    point /= dN;
    return point;
}


void Polygon2::enlarge(double fltDist)
{
    removeDegenerateVertexs();
    if(m_vecVertices.size() < 3u)        return;

    const double fltArea = area();
    if(floatEqual(fltArea, 0.0))        return;
    if(fltArea < 0.0)
    {
        reverseOrder();
    }

    std::vector<Point2d> vecNewVertexs;

    std::vector<Point2d>::iterator itorVertex0 = m_vecVertices.end() - 1u;
    std::vector<Point2d>::iterator itorVertex1 = m_vecVertices.begin();
    while(itorVertex1 != m_vecVertices.end())
    {
        Point2d &point0 = *itorVertex0;
        Point2d &point1 = *itorVertex1;

        Point2d  vecDir(point1.y() - point0.y(), point0.x() - point1.x());
        vecDir.normalize();
        vecDir *= fltDist;

        const Point2d pt0 = point0 + vecDir;
        const Point2d pt1 = pt0 + point1 - point0;
        vecNewVertexs.push_back(pt0);
        vecNewVertexs.push_back(pt1);

        itorVertex0 = itorVertex1;
        ++itorVertex1;
    }
    m_vecVertices = vecNewVertexs;
}


void Polygon2::insertVertexAfter(unsigned iInsertAfter, const Point2d &point)
{
    std::vector<Point2d>::iterator itorPos = m_vecVertices.begin() + iInsertAfter;
    if(iInsertAfter >= m_vecVertices.size())
    {
        itorPos = m_vecVertices.end();
    }
    m_vecVertices.insert(itorPos, point);
}


void Polygon2::clear(void)
{
    m_vecVertices.clear();
}


void Polygon2::addVertex(const Point2d &point)
{
    m_vecVertices.push_back(point);
}


void Polygon2::removeVertex(unsigned i)
{
    m_vecVertices.erase(m_vecVertices.begin() + i);
}


void Polygon2::reverseOrder(void)
{
    std::reverse(m_vecVertices.begin(), m_vecVertices.end());
}


unsigned Polygon2::removeDegenerateVertexs(double dEpsilon)
{
    if(m_vecVertices.size() <= 1u)    return 0u;

    unsigned nRemovedCount = 0u;
    std::vector<Point2d>::iterator itorVertex0 = m_vecVertices.end() - 1u;
    std::vector<Point2d>::iterator itorVertex1 = m_vecVertices.begin();
    while(itorVertex1 != m_vecVertices.end())
    {
        const Point2d diff = *itorVertex1 - *itorVertex0;
        if (fabs(diff.x()) < dEpsilon && fabs(diff.y()) < dEpsilon)
        {
            itorVertex1 = m_vecVertices.erase(itorVertex1);
            continue;
        }

        itorVertex0 = itorVertex1;
        ++itorVertex1;
    }

    return nRemovedCount;
}


bool Polygon2::isConvex(void) const
{
    if(m_vecVertices.size() <= 3u)    return true;

    unsigned positive = 0u;
    unsigned negative = 0u;
    for(unsigned int i = 0u; i < m_vecVertices.size(); i++)
    {
        const Point2d &p0 = getSafeVertex(i);
        const Point2d &p1 = getSafeVertex(i + 1u);
        const Point2d &p2 = getSafeVertex(i + 2u);

        const Point2d v1 = p1 - p0;
        const Point2d v2 = p2 - p1;

        const double cross = (v1.x() * v2.y() - v1.y() * v2.x());
        if(cross < 0.0)
            negative++;
        else
            positive++;
    }
    return (negative == 0u || positive == 0u);
}


const Point2d &Polygon2::getSafeVertex(unsigned index) const
{
    const unsigned points = m_vecVertices.size();
    if(index >= points)
    {
        return m_vecVertices[index % points];
    }
    return m_vecVertices[index];
}


void Polygon2::setSafeVertex(unsigned index, const Point2d &point)
{
    const unsigned points = m_vecVertices.size();
    if(index >= points)
    {
        m_vecVertices[index % points] = point;
    }
    else
    {
        m_vecVertices[index] = point;
    }
}


Box2d Polygon2::getBound(void) const
{
    if(m_vecVertices.size() < 1u)
    {
        return Box2d();
    }
    else if(m_vecVertices.size() < 2u)
    {
        return Box2d(m_vecVertices[0], m_vecVertices[0]);
    }

    Box2d bb(m_vecVertices[0], m_vecVertices[1]);
    for(unsigned int i = 2; i < m_vecVertices.size(); i++)
    {
        bb.expandBy(m_vecVertices[i]);
    }
    return bb;
}


bool Polygon2::isValid(void) const
{
    return (getVerticesCount() >= 3u);
}


bool Polygon2::isIntersectional(const Polygon2 &plg) const
{
    if(this == &plg)    return true;

    // 1、判定这两个多边形的矩形包围盒是否有重叠的可能
    // 1.1、找出传入的多边形的矩形包围盒
    Point2d ptMin1( FLT_MAX,  FLT_MAX);
    Point2d ptMax1(-FLT_MAX, -FLT_MAX);
    std::vector<Point2d>::const_iterator itorVertex = plg.m_vecVertices.begin();
    for(; itorVertex != plg.m_vecVertices.end(); ++itorVertex)
    {
        const Point2d &ptVertex = *itorVertex;
        if(ptVertex.x() < ptMin1.x())    ptMin1.x() = ptVertex.x();
        if(ptVertex.x() > ptMax1.x())    ptMax1.x() = ptVertex.x();
        if(ptVertex.y() < ptMin1.y())    ptMin1.y() = ptVertex.y();
        if(ptVertex.y() > ptMax1.y())    ptMax1.y() = ptVertex.y();
    }

    // 1.2、找出本多边形的矩形包围盒
    Point2d ptMin2( FLT_MAX,  FLT_MAX);
    Point2d ptMax2(-FLT_MAX, -FLT_MAX);
    for(itorVertex = m_vecVertices.begin(); itorVertex != m_vecVertices.end(); ++itorVertex)
    {
        const Point2d &ptVertex = *itorVertex;
        if(ptVertex.x() < ptMin2.x())    ptMin2.x() = ptVertex.x();
        if(ptVertex.x() > ptMax2.x())    ptMax2.x() = ptVertex.x();
        if(ptVertex.y() < ptMin2.y())    ptMin2.y() = ptVertex.y();
        if(ptVertex.y() > ptMax2.y())    ptMax2.y() = ptVertex.y();
    }

    // 1.3、判定它们是否有可能重叠，包围盒都不重叠，那么两多边形肯定不相交
    if(ptMax1.x() < ptMin2.x())    return false;
    if(ptMin1.x() > ptMax2.x())    return false;
    if(ptMax1.y() < ptMin2.y())    return false;
    if(ptMin1.y() > ptMax2.y())    return false;


    // 2、判断互相是否包含对方的顶点
    // 2.1、判定本多边形是否包含传入多边形的顶点
    for(itorVertex = plg.m_vecVertices.begin(); itorVertex != plg.m_vecVertices.end(); ++itorVertex)
    {
        const Point2d &ptVertex = *itorVertex;
        if(containsPoint(ptVertex))
        {
            return true;
        }
    }

    // 2.2、判定传入多边形是否包含本多边形的点
    for(itorVertex = m_vecVertices.begin(); itorVertex != m_vecVertices.end(); ++itorVertex)
    {
        const Point2d &ptVertex = *itorVertex;
        if(plg.containsPoint(ptVertex))
        {
            return true;
        }
    }


    // 3、判定互相是否和对方的边相交
    std::vector<Point2d>    vecVertexs0(m_vecVertices), vecVertexs1(plg.m_vecVertices);
    {
        //    注意：边相交是一个对浮点精度要求比较高的运算，需要尽量用小数值的浮点数
        const Point2d ptCentroid  = (centroid() + plg.centroid()) * 0.5f;
        std::vector<Point2d>::iterator itor;
        for(itor = vecVertexs0.begin(); itor != vecVertexs0.end(); ++itor)
        {
            itor->x() -= ptCentroid.x();
            itor->y() -= ptCentroid.y();
        }

        for(itor = vecVertexs1.begin(); itor != vecVertexs1.end(); ++itor)
        {
            itor->x() -= ptCentroid.x();
            itor->y() -= ptCentroid.y();
        }
    }

    std::vector<Point2d>::const_iterator itorVertex1 = vecVertexs0.end() - 1;
    std::vector<Point2d>::const_iterator itorVertex2 = vecVertexs0.begin();
    while(itorVertex2 != vecVertexs0.end())
    {
        const Point2d &ptVertex1 = *itorVertex1;
        const Point2d &ptVertex2 = *itorVertex2;

        const double X2_X1 = ptVertex2.x() - ptVertex1.x();
        const double Y2_Y1 = ptVertex2.y() - ptVertex1.y();
        const double X1Y2  = ptVertex1.x() * ptVertex2.y();
        const double X2Y1  = ptVertex2.x() * ptVertex1.y();

        std::vector<Point2d>::const_iterator itorVertex3 = vecVertexs1.end() - 1;
        std::vector<Point2d>::const_iterator itorVertex4 = vecVertexs1.begin();
        while(itorVertex4 != vecVertexs1.end())
        {
            const Point2d &ptVertex3 = *itorVertex3;
            const Point2d &ptVertex4 = *itorVertex4;

            const double X4_X3 = ptVertex4.x() - ptVertex3.x();
            const double Y4_Y3 = ptVertex4.y() - ptVertex3.y();
            const double X3Y4  = ptVertex3.x() * ptVertex4.y();
            const double X4Y3  = ptVertex4.x() * ptVertex3.y();

            const double Den = X2_X1 * Y4_Y3 - X4_X3 * Y2_Y1;

            const double NumX = X2_X1 * (X3Y4 - X4Y3) - X4_X3 * (X1Y2 - X2Y1);
            const double X = NumX / Den;

            const double NumY = Y2_Y1 * (X3Y4 - X4Y3) - Y4_Y3 * (X1Y2 - X2Y1);
            const double Y = NumY / Den;

            bool bInter = cmm::math::isValueInArea(ptVertex1.x(), ptVertex2.x(), X);
            bInter = (bInter && cmm::math::isValueInArea(ptVertex1.y(), ptVertex2.y(), Y));
            bInter = (bInter && cmm::math::isValueInArea(ptVertex3.x(), ptVertex4.x(), X));
            bInter = (bInter && cmm::math::isValueInArea(ptVertex3.y(), ptVertex4.y(), Y));

            if(bInter)    return true;

            itorVertex3 = itorVertex4;
            ++itorVertex4;
        }

        itorVertex1 = itorVertex2;
        ++itorVertex2;
    }

    return  false;
}


}}