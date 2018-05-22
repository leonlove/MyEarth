#include "TerrainDemModification.h"
#include <algorithm>
#include <osgDB/ReadFile>
#include <IDProvider/Definer.h>

TerrainDemModification::TerrainDemModification(const std::string &strType, ea::IEventAdapter *pEventAdapter)
    : TerrainModification(strType, pEventAdapter)
{
}


TerrainDemModification::~TerrainDemModification(void)
{
}


bool TerrainDemModification::setDemFile(const std::string &strFilePath)
{
    if(isApply())   return false;
    if(strFilePath.empty()) return false;

    std::string strFile = strFilePath;
    std::transform(strFile.begin(), strFile.end(), strFile.begin(), ::toupper);
    if(strFile == m_strDemFile)
    {
        return true;
    }
    m_strDemFile = strFile;
    m_pDemImage = osgDB::readImageFile(m_strDemFile);
    return true;
}


const std::string &TerrainDemModification::getDemFile(void) const
{
    return m_strDemFile;
}


bool TerrainDemModification::modifyTerrainTile(osg::Node *pTerrainTile) const
{
    if(!isApply())  return false;

    if(!m_pDemImage.valid())
    {
        return false;
    }
    osgTerrain::TerrainTile *pTT = dynamic_cast<osgTerrain::TerrainTile *>(pTerrainTile);
    if(pTerrainTile == NULL)
    {
        return false;
    }

    osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
    osg::HeightField *pHF = pHFLayer->getHeightField();

    const double fXInterval = pHF->getXInterval();
    const double fYInterval = pHF->getYInterval();
    const unsigned int nX = pHF->getNumRows();
    const unsigned int nY = pHF->getNumColumns();
    const osg::Vec3d &vTileMin = pHF->getOrigin();
    osg::Vec3d vTileMax(vTileMin._v[0] + (nX - 1) * fXInterval, vTileMin._v[1] + (nY - 1) * fYInterval, vTileMin._v[2]);
    const cmm::math::Box2d TileBB(cmm::math::Point2d(vTileMin._v[0], vTileMin._v[1]), cmm::math::Point2d(vTileMax._v[0], vTileMax._v[1]));

    const cmm::math::Point2d boxMin = m_Box.corner(cmm::math::Box2d::LeftBottom);
    const cmm::math::Point2d boxMax = m_Box.corner(cmm::math::Box2d::RightTop);
    if(!m_Box.contain(TileBB))
    {
        return false;
    }

    const double fSrcXRatio = (boxMax._v[0] - boxMin._v[0]) / m_pDemImage->s();
    const double fSrcYRatio = (boxMax._v[1] - boxMin._v[1]) / m_pDemImage->t();

    osg::ref_ptr<osg::Image> pCopyImage = new osg::Image(*(m_pDemImage.get()));
    const unsigned int nNewWidth = m_pDemImage->s() / (fXInterval / fSrcXRatio);
    const unsigned int nNewHeight = m_pDemImage->t() / (fYInterval / fSrcYRatio);
    pCopyImage->scaleImage(nNewWidth, nNewHeight, 1, m_pDemImage->getDataType());
    unsigned char *pCopyData = (unsigned char *)pCopyImage->getDataPointer();

    return true;
}

