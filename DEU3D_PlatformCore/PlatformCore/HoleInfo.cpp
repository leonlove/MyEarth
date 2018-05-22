#include "HoleInfo.h"
#include <osgUtil/Tessellator>

osg::Geometry *createSheetWithHoles(const std::vector<osg::Vec3d> &vecVertices, const HoleInfoList &listHoleInfo)
{
    osg::ref_ptr<osg::Geometry>     pTessGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array>    pTessVtxArray = new osg::Vec3Array;
    pTessGeometry->setVertexArray(pTessVtxArray.get());

    pTessVtxArray->assign(vecVertices.begin(), vecVertices.end());
    unsigned nLength = pTessVtxArray->size();

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, nLength);
    pTessGeometry->addPrimitiveSet(pDrawArrays.get());

    for(HoleInfoList::const_iterator itorHole = listHoleInfo.begin(); itorHole != listHoleInfo.end(); ++itorHole)
    {
        const HoleInfo &hole = *itorHole;

        std::vector<osg::Vec3d> vecHoleVertices;
        Hole2Vertices(hole, vecHoleVertices);

        pTessVtxArray->insert(pTessVtxArray->end(), vecHoleVertices.begin(), vecHoleVertices.end());

        osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::POLYGON, nLength, vecHoleVertices.size());
        pTessGeometry->addPrimitiveSet(pDrawArrays.get());

        nLength += vecHoleVertices.size();
    }

    osg::ref_ptr<osgUtil::Tessellator> pTessellator = new osgUtil::Tessellator;
    pTessellator->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    pTessellator->setBoundaryOnly(false);
    pTessellator->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
    pTessellator->retessellatePolygons(*pTessGeometry);

    return pTessGeometry.release();
}

osg::Geometry *createCircleWithHoles(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint, const HoleInfoList &listHoleInfo)
{
    std::vector<osg::Vec3d> vecVertices;
    genCircleVertices(ptCenter, dblRadius, nHint, false, false, vecVertices);

    return createSheetWithHoles(vecVertices, listHoleInfo);
}

void Hole2Vertices(const HoleInfo &hole, std::vector<osg::Vec3d> &vecVertices)
{
    if(!hole.m_bCircleHole)
    {
        vecVertices = hole.m_vecPolygon;
        return;
    }

    vecVertices.clear();
    genCircleVertices(hole.m_ptCenter, hole.m_dblRadius, hole.m_nHint, false, false, vecVertices);
}

void genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint, bool bClockWise, bool bClosed, std::vector<osg::Vec3d> &vecVertices)
{
    const unsigned nEdgeCount = osg::clampAbove(nHint, 3u);
    const double dblDeltaTheta = (bClockWise ? (-osg::PI * 2.0 / nEdgeCount) : (osg::PI * 2.0 / nEdgeCount));
    double dblTheta = 0.0;
    const unsigned nLoop = (bClosed ? nEdgeCount + 1 : nEdgeCount);
    for(unsigned n = 0u; n < nLoop; n++)
    {
        osg::Vec3d point;
        point.x() = ptCenter.x() + cos(dblTheta) * dblRadius;
        point.y() = ptCenter.y() + sin(dblTheta) * dblRadius;
        point.z() = ptCenter.z();
        vecVertices.push_back(point);

        dblTheta += dblDeltaTheta;
    }
}