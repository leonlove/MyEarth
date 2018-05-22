#include "VirtualTileReaderWriter.h"

#include <strstream>
#include <OpenSP/op.h>
#include <Common/DEUBson.h>
#include <Common/Common.h>
#include <osg/CoordinateSystemNode>
#include <osg/PagedLOD>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <IDProvider/Definer.h>
#include <sstream>


VirtualTileReaderWriter::VirtualTileReaderWriter(const vtm::IVirtualTileManager *pVTileManager) :
    m_pVTileManager(pVTileManager)
{
    const double dblRangeRatio = pVTileManager->getTileRangeRatio();
    m_pLODFixer = new LODFixer(dblRangeRatio);

#if DUMP_V_TILE_LOG
    m_fileLogOut.open("c:\\vTile_trunk.log");
#endif
}


VirtualTileReaderWriter::~VirtualTileReaderWriter(void)
{
    m_pLODFixer = NULL;
    m_pVTileManager = NULL;

#if DUMP_V_TILE_LOG
    m_fileLogOut.close();
#endif
}


static osg::Node *createQuad(const osg::Vec2d &coordMin, const osg::Vec2d &coordMax, const osg::Vec4 &color)
{
    osg::Vec2d coord0 = coordMin;
    osg::Vec2d coord1 = coordMax;
    const osg::Vec2d vecBias(coord1 - coord0);
    coord0 += vecBias * 0.02;
    coord1 -= vecBias * 0.02;

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;

    osg::Vec3d point;
    pEllipsoidModel->convertLatLongHeightToXYZ(coord0.y(), coord0.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord0.y(), coord1.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord1.y(), coord1.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord1.y(), coord0.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);

    pEllipsoidModel->convertLatLongHeightToXYZ(coord0.y(), coord0.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord1.y(), coord1.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord1.y(), coord0.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);
    pEllipsoidModel->convertLatLongHeightToXYZ(coord0.y(), coord1.x(), 0.0, point.x(), point.y(), point.z());
    pVtxArray->push_back(point);

    pGeometry->setVertexArray(pVtxArray.get());

    osg::ref_ptr<osg::Vec4Array>    pColorArray = new osg::Vec4Array;
    pColorArray->push_back(color);
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(GL_LINE_STRIP, 0u, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pGeometry.get());
    return pGeode.release();
};


osgDB::ReaderWriter::ReadResult VirtualTileReaderWriter::readNode(const ID &id, const VTileData &pairVTData, const osgDB::Options* options) const
{
    bool vecValid[4] = {false, false, false, false};

    ObjectMatrix    mtxObjectIDs;
    readObjects(id, pairVTData, vecValid, mtxObjectIDs);

#if DUMP_V_TILE_LOG
    {
        std::ostringstream oss;
        oss << (unsigned )id.TileID.m_nLevel << ' '
            << (unsigned)id.TileID.m_nRow    << ' '
            << (unsigned)id.TileID.m_nCol    << ' ';

        const unsigned nFragCount = m_pVTileManager->getTileFragmentCount();
        for(unsigned y = 0u; y < nFragCount; y++)
        {
            for(unsigned x = 0u; x < nFragCount; x++)
            {
                IDList &idList = mtxObjectIDs[y][x];
                oss << idList.size() << ' ';
            }
        }
        oss << std::endl;
        m_fileLogOut << oss.str().c_str();
        m_fileLogOut.flush();
    }
#endif

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    pGroup->setID(id);

    const unsigned nLevel = id.TileID.m_nLevel;
    const unsigned nRow = id.TileID.m_nRow;
    const unsigned nCol = id.TileID.m_nCol;

    const double dblTileRangeRatio = m_pVTileManager->getTileRangeRatio();

    //创建下面的PagedLOD子树
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const unsigned nPagedLODCount = ((nLevel == 0u) ? 2u : 4u);
    for(unsigned i = 0u; i < nPagedLODCount; i++)
    {
        const unsigned nChildLevel = nLevel + 1u;
        const unsigned nChildCol   = (nCol << 1u) + i % 2u;
        const unsigned nChildRow   = (nRow << 1u) + (i / 2u) % 2u;

        //获取此地块的范围，单位为经纬度
        osg::Vec2d ptMinCoord, ptMaxCoord;
        pPyramid->getTilePos(nChildLevel, nChildRow, nChildCol, ptMinCoord.x(), ptMinCoord.y(), ptMaxCoord.x(), ptMaxCoord.y());

        //获取此地块的中心点，单位为经纬度
        const osg::Vec2d ptCenterCoord((ptMinCoord + ptMaxCoord) * 0.5);

        osg::Vec3d ptMinPos, ptMaxPos, ptCenterPos;
        pEllipsoidModel->convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), 0.0, ptMinPos.x(), ptMinPos.y(), ptMinPos.z());
        pEllipsoidModel->convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), 0.0, ptMaxPos.x(), ptMaxPos.y(), ptMaxPos.z());
        pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), 0.0, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

        //计算地块的半径
        const double dblRadius1 = (ptMinPos - ptMaxPos).length() * 0.5;
        const double dblRadius2 = (ptCenterPos - ptMinPos).length();
        const double dblRadius3 = (ptCenterPos - ptMaxPos).length();
        const double dblRadius  = std::max(dblRadius1, std::max(dblRadius2, dblRadius3));

        osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
        const ID child_id(0u, nChildLevel, nChildRow, nChildCol, 0);
        pPagedLOD->setFileID(0u, child_id);
        pPagedLOD->setRange(0u, 0.0, dblRadius * dblTileRangeRatio);
        pPagedLOD->setCenter(ptCenterPos);
        pPagedLOD->setRadius(dblRadius);

        if(!vecValid[i])
        {
            pPagedLOD->setNodeMask(0);
        }
        pGroup->addChild(pPagedLOD.get());
    }

    osg::ref_ptr<DEUProxyGroup> pProxyGroup = new DEUProxyGroup;
    pProxyGroup->create(m_pVTileManager.get(), id, mtxObjectIDs, m_pLODFixer.get());
    pGroup->addChild(pProxyGroup.get());

    //osg::Vec2d ptMinCoord, ptMaxCoord;
    //pPyramid->getTilePos(nLevel, nRow, nCol, ptMinCoord.x(), ptMinCoord.y(), ptMaxCoord.x(), ptMaxCoord.y());
    //const osg::Vec2d ptCenterCoord((ptMinCoord + ptMaxCoord) * 0.5);

    //osg::Vec3d ptMinPos, ptMaxPos, ptCenterPos;
    //pEllipsoidModel->convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), 0.0, ptMinPos.x(), ptMinPos.y(), ptMinPos.z());
    //pEllipsoidModel->convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), 0.0, ptMaxPos.x(), ptMaxPos.y(), ptMaxPos.z());
    //pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), 0.0, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());
    //const double dblRadius1 = (ptMinPos - ptMaxPos).length() * 0.5;
    //const double dblRadius2 = (ptCenterPos - ptMinPos).length();
    //const double dblRadius3 = (ptCenterPos - ptMaxPos).length();
    //const double dblRadius  = std::max(dblRadius1, std::max(dblRadius2, dblRadius3));

    //if(nLevel >= 14 && nLevel <= 16)
    //{
    //    osg::ref_ptr<osg::Node>   pDebugNode = createQuad(ptMinCoord, ptMaxCoord, osg::Vec4(1.0, 0.2, 0.2, 1.0));
    //    osg::LineWidth *pLineWidth = new osg::LineWidth(3);
    //    pDebugNode->getOrCreateStateSet()->setAttributeAndModes(pLineWidth);
    //    pGroup->addChild(pDebugNode);
    //}

    return pGroup.release();
}


void VirtualTileReaderWriter::readObjects(const ID &id, const VTileData &pairVTData, bool vecValid[], ObjectMatrix &mtxObjectIDs) const
{
    mtxObjectIDs.clear();

    vecValid[0] = false;
    vecValid[1] = false;
    vecValid[2] = false;
    vecValid[3] = false;

    const unsigned nFragCount = m_pVTileManager->getTileFragmentCount();
    mtxObjectIDs.resize(nFragCount);
    for(unsigned y = 0u; y < nFragCount; y++)
    {
        mtxObjectIDs[y].resize(nFragCount);
    }

    OpenSP::sp<vtm::IVirtualTile>   pRemoteVTile = NULL;
    const void *pData = pairVTData.first;
    unsigned int nLen = pairVTData.second;
    if(pData != NULL && nLen > 0u)
    {
        pRemoteVTile = vtm::createVirtualTileByStream(pData, nLen);
    }

    OpenSP::sp<vtm::IVirtualTile> pVTile = m_pVTileManager->copyVirtualTile(id);
    if(pVTile)
    {
        if(pRemoteVTile.valid())
        {
            pVTile->mergeVTile(pRemoteVTile);
        }
    }
    else
    {
        if(pRemoteVTile.valid())
        {
            pVTile = pRemoteVTile;
        }
    }

    if(!pVTile.valid()) return;

    pVTile->getChildTileState(vecValid);

    for(unsigned y = 0u; y < nFragCount; y++)
    {
        for(unsigned x = 0u; x < nFragCount; x++)
        {
            IDList &idList = mtxObjectIDs[y][x];
            pVTile->getFragmentObjects(y, x, idList);
        }
    }
}


