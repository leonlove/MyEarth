#include "TerrainElevationModification.h"

#include <common/IDEUImage.h>
#include <common/Pyramid.h>
#include <IDProvider/Definer.h>
#include <osg/ClusterCullingCallback>
#include <osgUtil/ClusterCullingCreator>

TerrainElevationModification::TerrainElevationModification(const std::string &strType, ea::IEventAdapter *pEventAdapter)
    : TerrainModification(strType, pEventAdapter)
{
    m_dblSmoothInterval = 16.0;
}


TerrainElevationModification::~TerrainElevationModification(void)
{
}


void TerrainElevationModification::setElevation(double dblElevation)
{
    if(isApply())   return;
    m_dblElevation = dblElevation;
}


double TerrainElevationModification::getElevation(void) const
{
    return m_dblElevation;
}


void TerrainElevationModification::setSmoothInterval(double dblSmooth)
{
    if(isApply())   return;
    m_dblSmoothInterval = osg::clampAbove(dblSmooth, 0.0);
}


double TerrainElevationModification::getSmoothInterval(void) const
{
    return m_dblSmoothInterval;
}


bool TerrainElevationModification::modifyTerrainTile(osg::Node *pTerrainTileNode) const
{
    if(!isApply())  return false;

    osgTerrain::TerrainTile *pTerrainTile = dynamic_cast<osgTerrain::TerrainTile *>(pTerrainTileNode);
    if(pTerrainTile == NULL)
    {
        return false;
    }

    osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTerrainTile->getElevationLayer());
    osg::HeightField *pHeightField = pHFLayer->getHeightField();
    if(pHeightField == NULL)
    {
        return false;
    }

    unsigned int nX = pHeightField->getNumRows();
    unsigned int nY = pHeightField->getNumColumns();
    if(nX < 2u || nY < 2u)
    {
        return false;
    }

    const osg::Vec2d vecIntervalOrg(pHeightField->getXInterval(), pHeightField->getYInterval());
    const cmm::math::Point2d ptTileMin(pHeightField->getOrigin().x(), pHeightField->getOrigin().y());
    const cmm::math::Point2d ptTileMax
    (
        ptTileMin.x() + (nX - 1) * vecIntervalOrg.x(),
        ptTileMin.y() + (nY - 1) * vecIntervalOrg.y()
    );

    cmm::math::Polygon2  polygonTile;
    polygonTile.addVertex(ptTileMin);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMax.x(), ptTileMin.y()));
    polygonTile.addVertex(ptTileMax);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMin.x(), ptTileMax.y()));

    if(!shouldBeModified(polygonTile, cmm::math::Box2d(ptTileMin, ptTileMax)))
    {
        return false;
    }

    pHFLayer->backup();

    OpenSP::sp<cmm::image::IDEUImage> pImage = cmm::image::createDEUImage();
    pImage->attach((float *)pHeightField->getFloatArray()->getDataPointer(), nX, nY, cmm::image::PF_LUMINANCE);
    OpenSP::sp<cmm::image::IDEUImage> pScaleImage = pImage;

    osg::Vec2d vecInterval = vecIntervalOrg;

    unsigned int nInterval = std::max(nX, nY);
    const ID &id = pTerrainTileNode->getID();

    if(id.TileID.m_nLevel > 10u && id.TileID.m_nLevel <= 12u)
    {
        nInterval = 16u;
    }
    else if(id.TileID.m_nLevel > 12u && id.TileID.m_nLevel <= 15u)
    {
        nInterval = 32u;
    }
    else if(id.TileID.m_nLevel > 15u)
    {
        nInterval = 64u;
    }

    //if(nX < 64u || nY < 64u)
    if(nX < nInterval || nY < nInterval)
    {
        nX = std::max(nInterval, nX);
        nY = std::max(nInterval, nY);

        pScaleImage = pImage->scaleImage(nX, nY);

        const ID &id = pTerrainTile->getID();
        double dXmin, dXmax, dYmin, dYmax;
        const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
        pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, dXmin, dYmin, dXmax, dYmax);

        const osg::Vec2d vecInterval0((dXmax - dXmin) / float(nX - 1u), (dYmax - dYmin) / float(nY - 1u));
        vecInterval.set(vecInterval0.x(), vecInterval0.y());

        pHeightField->allocate(nX, nY);
        pHeightField->setXInterval(vecInterval0.x());
        pHeightField->setYInterval(vecInterval0.y());
    }

    const bool bShouldSmooth = !cmm::math::floatEqual(m_dblSmoothInterval, 0.0);
    bool bAllModified = true;
    float *pData = (float *)pScaleImage->data();
    for(unsigned int k = 0; k < nY; k++)
    {
        for(unsigned int j = 0; j < nX; j++)
        {
            const cmm::math::Point2d vtx(ptTileMin.x() + j * vecInterval.x(), ptTileMin.y() + k * vecInterval.y());

            if(m_Polygon.containsPoint(vtx))
            {
                *pData = m_dblElevation;
            }
            else
            {
                if(bShouldSmooth)
                {
                    const double dblDistance = calcDistanceOnEarth(m_Polygon, vtx);
                    if(dblDistance < m_dblSmoothInterval)
                    {
                        const double dblTemp  = dblDistance / m_dblSmoothInterval;
                        const double dblRatio = cmm::math::sinPress(dblTemp, 1u);
                        const double dblData  = *pData;
                        *pData = m_dblElevation * (1.0 - dblRatio) + dblData * dblRatio;
                    }
                    else
                    {
                        bAllModified = false;
                    }
                }
                else
                {
                    bAllModified = false;
                }
            }

            pData++;
        }
    }

    pData = (float *)pScaleImage->data();
    osg::FloatArray *pFloatArray = pHeightField->getFloatArray();
    pFloatArray->assign(pData, pData + nX * nY);

    if(!bAllModified)
    {
        const double dblSkirt = std::max((double)pHeightField->getSkirtHeight(), fabs(m_dblElevation));
        pHeightField->setSkirtHeight(dblSkirt);
    }

    pTerrainTile->dirtyBound();
    pHFLayer->dirty();
    const osg::BoundingSphere &bound = pTerrainTile->getBound();
    pTerrainTile->computeBound();
    for(unsigned n = 0u; n < pTerrainTile->getNumParents(); n++)
    {
        osg::LOD *pLod = dynamic_cast<osg::LOD *>(pTerrainTile->getParent(n));
        if(!pLod)   continue;
        pLod->setCenter(bound.center());
    }

    osg::ref_ptr<osg::ClusterCullingCallback> pClusterCallback = osgUtil::createClusterCullingCallbackByHeightField(pHeightField);
    pTerrainTile->addCullCallback(pClusterCallback.get());
    return true;
}


bool TerrainElevationModification::shouldBeModified(const cmm::math::Polygon2 &polygonTile, const cmm::math::Box2d &bbTile) const
{
    if(TerrainModification::shouldBeModified(polygonTile, bbTile))
    {
        return true;
    }

    if(cmm::math::floatEqual(m_dblSmoothInterval, 0.0))
    {
        return false;
    }

    for(unsigned n = 0u; n < m_Polygon.getVerticesCount(); n++)
    {
        const cmm::math::Point2d &vtx = m_Polygon.getSafeVertex(n);
        const double dbl = calcDistanceOnEarth(polygonTile, vtx);
        if(dbl < m_dblSmoothInterval)
        {
            return true;
        }
    }

    for(unsigned n = 0u; n < polygonTile.getVerticesCount(); n++)
    {
        const cmm::math::Point2d &vtx = polygonTile.getSafeVertex(n);
        const double dbl = calcDistanceOnEarth(m_Polygon, vtx);
        if(dbl < m_dblSmoothInterval)
        {
            return true;
        }
    }

    return false;
}


double TerrainElevationModification::calcDistanceOnEarth(const cmm::math::Polygon2 &polygon, const cmm::math::Point2d &ptTest) const
{
    cmm::math::Point2d vtx0, vtx1;
    const double dbl = polygon.findNearestSegment(ptTest, vtx0, vtx1);
    const double dbl_0 = (ptTest - vtx0).length();
    const double dbl_1 = (ptTest - vtx1).length();

    double dblDistance = 0.0;
    if(dbl < dbl_0 && dbl < dbl_1)
    {
        // 计算ptTest到垂足的球面距离
        cmm::math::Vector2d vec1 = vtx1 - vtx0;
        const double dblNormal = vec1.normalize();
        if(cmm::math::floatEqual(dblNormal, 0.0))
        {
            return calcDistanceOnEarth(ptTest, vtx0);
        }

        const cmm::math::Vector2d vec2 = ptTest - vtx0;
        const double dbl = vec1 * vec2;
        const cmm::math::Point2d pt = vtx0 + vec1 * dbl;
        return calcDistanceOnEarth(ptTest, pt);
    }

    if(dbl_0 < dbl_1)
    {
        // 计算ptTest到vtx0的球面距离
        return calcDistanceOnEarth(ptTest, vtx0);
    }
    else
    {
        // 计算ptTest到vtx1的球面距离
        return calcDistanceOnEarth(ptTest, vtx1);
    }
}


double TerrainElevationModification::calcDistanceOnEarth(const cmm::math::Point2d &point0, const cmm::math::Point2d &point1) const
{
    // 本函数计算的实际不是球面距离，只是一个近似值而已

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    cmm::math::Point3d pt0;
    pEllipsoidModel->convertLatLongHeightToXYZ(point0.y(), point0.x(), 0.0, pt0.x(), pt0.y(), pt0.z());

    cmm::math::Point3d pt1;
    pEllipsoidModel->convertLatLongHeightToXYZ(point1.y(), point1.x(), 0.0, pt1.x(), pt1.y(), pt1.z());

    return (pt0 - pt1).length();
}


