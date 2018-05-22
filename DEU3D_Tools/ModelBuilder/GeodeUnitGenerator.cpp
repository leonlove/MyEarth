#include "GeodeUnitGenerator.h"
#include <osg/Geometry>


GeodeUnitGenerator::GeodeUnitGenerator(void)
{
    m_dblBubbleRadius = 25.0;
    m_pBubbleCenters = new osg::Vec3dArray;
    m_pGeodeVertices = new osg::Vec3dArray;
}


GeodeUnitGenerator::~GeodeUnitGenerator(void)
{
}


const osg::Vec3dArray *GeodeUnitGenerator::findBubbleCenters(osg::Geode &node)
{
    m_pGeodeVertices->clear();
    const unsigned nGeomCount = node.getNumDrawables();
    for(unsigned n = 0u; n < nGeomCount; n++)
    {
        osg::Drawable *pDrawable = node.getDrawable(n);
        osg::Geometry *pGeom = dynamic_cast<osg::Geometry *>(pDrawable);
        if(!pGeom)  continue;

        const osg::Array *pArray = pGeom->getVertexArray();
        const osg::Vec3Array *pFloatArray = dynamic_cast<const osg::Vec3Array *>(pArray);
        if(pFloatArray)
        {
            m_pGeodeVertices->insert(m_pGeodeVertices->end(), pFloatArray->begin(), pFloatArray->end());
        }
        else
        {
            const osg::Vec3dArray *pDoubleArray = dynamic_cast<const osg::Vec3dArray *>(pArray);
            if(pDoubleArray)
            {
                m_pGeodeVertices->insert(m_pGeodeVertices->end(), pDoubleArray->begin(), pDoubleArray->end());
            }
        }
    }

    const osg::BoundingBox &bb = node.getBoundingBox();
    m_GeodeBoundingBox.expandBy(bb._max);
    m_GeodeBoundingBox.expandBy(bb._min);

    const_cast<osg::Vec3dArray *>(m_pBubbleCenters.get())->clear();
    traverseBoundingBox_X();
    return m_pBubbleCenters.get();
}


void GeodeUnitGenerator::traverseBoundingBox_X(void) const
{
    const osg::Vec3d center = m_GeodeBoundingBox.center();
    const double dblBubbleDiameter = m_dblBubbleRadius + m_dblBubbleRadius;

    const double dblMaxX = m_GeodeBoundingBox.xMax() + dblBubbleDiameter;
    for(double x = center.x(); x < dblMaxX; x += dblBubbleDiameter)
    {
        traverseBoundingBox_Y(x);
    }

    const double dblMinX = m_GeodeBoundingBox.xMin() - dblBubbleDiameter;
    for(double x = center.x() - dblBubbleDiameter; x > dblMinX; x -= dblBubbleDiameter)
    {
        traverseBoundingBox_Y(x);
    }
}


void GeodeUnitGenerator::traverseBoundingBox_Y(double x) const
{
    const osg::Vec3d center = m_GeodeBoundingBox.center();
    const double dblBubbleDiameter = m_dblBubbleRadius + m_dblBubbleRadius;

    const double dblMaxY = m_GeodeBoundingBox.yMax() + dblBubbleDiameter;
    for(double y = center.y(); y < dblMaxY; y += dblBubbleDiameter)
    {
        traverseBoundingBox_Z(x, y);
    }

    const double dblMinY = m_GeodeBoundingBox.yMin() - dblBubbleDiameter;
    for(double y = center.y() - dblBubbleDiameter; y > dblMinY; y -= dblBubbleDiameter)
    {
        traverseBoundingBox_Z(x, y);
    }
}


void GeodeUnitGenerator::traverseBoundingBox_Z(double x, double y) const
{
    const osg::Vec3d center = m_GeodeBoundingBox.center();
    const double dblBubbleDiameter = m_dblBubbleRadius + m_dblBubbleRadius;
    const double dblMaxZ = m_GeodeBoundingBox.zMax() + dblBubbleDiameter;
    for(float z = center.z(); z < dblMaxZ; z += dblBubbleDiameter)
    {
        const osg::Vec3 ptMin(x - m_dblBubbleRadius, y - m_dblBubbleRadius, z - m_dblBubbleRadius);
        const osg::Vec3 ptMax(x + m_dblBubbleRadius, y + m_dblBubbleRadius, z + m_dblBubbleRadius);
        const osg::BoundingBoxd box(ptMin, ptMax);
        if(validBubble(box))
        {
            m_pBubbleCenters->push_back(osg::Vec3d(x, y, z));
        }
    }

    const double dblMinZ = m_GeodeBoundingBox.zMin() - dblBubbleDiameter;
    for(float z = center.z() - dblBubbleDiameter; z > dblMinZ; z -= dblBubbleDiameter)
    {
        const osg::Vec3 ptMin(x - m_dblBubbleRadius, y - m_dblBubbleRadius, z - m_dblBubbleRadius);
        const osg::Vec3 ptMax(x + m_dblBubbleRadius, y + m_dblBubbleRadius, z + m_dblBubbleRadius);
        const osg::BoundingBoxd box(ptMin, ptMax);
        if(validBubble(box))
        {
            m_pBubbleCenters->push_back(osg::Vec3(x, y, z));
        }
    }
}


bool GeodeUnitGenerator::validBubble(const osg::BoundingBoxd &box) const
{
    osg::Vec3dArray::const_iterator itorVtx = m_pGeodeVertices->begin();
    for( ; itorVtx != m_pGeodeVertices->end(); ++itorVtx)
    {
        const osg::Vec3d &vtx = *itorVtx;
        if(box.contains(vtx))
        {
            return true;
        }
    }

    return false;
}

