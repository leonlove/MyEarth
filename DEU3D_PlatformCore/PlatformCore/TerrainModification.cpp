#include "TerrainModification.h"
#include <EventAdapter/IEventObject.h>

TerrainModification::TerrainModification(const std::string &strType, ea::IEventAdapter *pEventAdapter) :
    m_strType(strType),
    m_pEventAdapter(pEventAdapter),
    m_bApply(false)
{
}


TerrainModification::~TerrainModification(void)
{
}


void TerrainModification::addVertex(double dblX, double dblY)
{
    if(isApply())   return;

    const cmm::math::Point2d point(dblX, dblY);
    m_Polygon.addVertex(point);
    m_Box = m_Polygon.getBound();
}


bool TerrainModification::removeVertex(unsigned nIndex)
{
    if(isApply())   return false;
    if(nIndex >= m_Polygon.getVerticesCount())
    {
        return false;
    }
    m_Polygon.removeVertex(nIndex);
    m_Box = m_Polygon.getBound();

    return true;
}


bool TerrainModification::shouldBeModified(const osgTerrain::TerrainTile *pTerrainTile) const
{
    const osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<const osgTerrain::HeightFieldLayer *>(pTerrainTile->getElevationLayer());
    const osg::HeightField *pHF = pHFLayer->getHeightField();

    const cmm::math::Point2d vecInterval(pHF->getXInterval(), pHF->getYInterval());
    const unsigned int nElevRows = pHF->getNumRows();
    const unsigned int nElevCols = pHF->getNumColumns();
    const cmm::math::Point2d ptTileMin(pHF->getOrigin().x(), pHF->getOrigin().y());
    const cmm::math::Point2d ptTileMax(ptTileMin.x() + (nElevCols - 1u) * vecInterval.x(), ptTileMin.y() + (nElevRows - 1u) * vecInterval.y());
    const cmm::math::Box2d TileBB(ptTileMin, ptTileMax);

    cmm::math::Polygon2  polygonTile;
    polygonTile.addVertex(ptTileMin);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMin.x(), ptTileMax.y()));
    polygonTile.addVertex(ptTileMax);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMax.x(), ptTileMin.y()));

    return shouldBeModified(polygonTile, TileBB);
}


bool TerrainModification::shouldBeModified(const cmm::math::Polygon2 &polygonTile, const cmm::math::Box2d &bbTile) const
{
    if(!m_Box.contain(bbTile))
    {
        return false;
    }

    if(m_Polygon.isIntersectional(polygonTile))
    {
        return true;
    }
    return false;
}


void TerrainModification::setApply(bool bApply)
{
    const unsigned nApply = (unsigned)m_bApply;
    if(!!nApply == !!bApply)  return;

    bApply ? m_bApply.exchange(1) : m_bApply.exchange(0);

    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
    pEventObject->setAction("RefreshTerrainTile");
    pEventObject->putExtra("Apply", bApply);

    cmm::variant_data varObj;
    OpenSP::Ref *pRefThis = this;
    varObj = pRefThis;
    pEventObject->putExtra("Object", varObj);

    m_pEventAdapter->sendBroadcast(pEventObject);
}