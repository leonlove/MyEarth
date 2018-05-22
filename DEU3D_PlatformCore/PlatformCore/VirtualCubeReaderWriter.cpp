#include "VirtualCubeReaderWriter.h"

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


VirtualCubeReaderWriter::VirtualCubeReaderWriter(const vcm::IVirtualCubeManager *pVCubeManager) :
    m_pVCubeManager(pVCubeManager)
{
    const double dblRangeRatio = pVCubeManager->getCubeRangeRatio();
    m_pLODFixer = new LODFixer(dblRangeRatio);
}


VirtualCubeReaderWriter::~VirtualCubeReaderWriter(void)
{
    m_pLODFixer = NULL;
    m_pVCubeManager = NULL;
}


osgDB::ReaderWriter::ReadResult VirtualCubeReaderWriter::readNode(const ID &id, const VCubeData &pairVTData, const osgDB::Options* options) const
{
    bool vecValid[8] = {false, false, false, false, false, false, false, false};

    ObjectCube    cubeObjectIDs;
    readObjects(id, pairVTData, vecValid, cubeObjectIDs);

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    pGroup->setID(id);

    const unsigned nLevel  = id.CubeID.m_nLevel;
    const unsigned nRow    = id.CubeID.m_nRow;
    const unsigned nCol    = id.CubeID.m_nCol;
    const unsigned nHeight = id.CubeID.m_nHeight;

    const double dblCubeRangeRatio = m_pVCubeManager->getCubeRangeRatio();

    //创建下面的PagedLOD子树
    const cmm::Pyramid3 *pPyramid = cmm::Pyramid3::instance();
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const unsigned nRowCount = (nLevel == 0u ? 1u : 2u);
    const unsigned nChildLevel = nLevel + 1u;
    unsigned nChildIndex = 0u;
    for(unsigned y = 0u; y < nRowCount; y++)
    {
        const unsigned nChildRow = (nRow << 1u) + y;
        for(unsigned x = 0u; x < 2u; x++)
        {
            const unsigned nChildCol = (nCol << 1u) + x;
            for(unsigned z = 0u; z < 2u; z++)
            {
                const unsigned nChildHeight = (nHeight << 1u) + z;

                cmm::math::Point3d ptMinCoord, ptMaxCoord;
                pPyramid->getCubePos(nChildLevel, nChildRow, nChildCol, nChildHeight, ptMinCoord, ptMaxCoord);
                cmm::math::Point3d ptCenterCoord = (ptMinCoord + ptMaxCoord) * 0.5;

                {
                    double dblSeaLevel;

                    dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptCenterCoord.y(), ptCenterCoord.x());
                    ptCenterCoord.z() -= dblSeaLevel;

                    dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptMinCoord.y(), ptMinCoord.x());
                    ptMinCoord.z() -= dblSeaLevel;

                    dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptMaxCoord.y(), ptMinCoord.x());
                    ptMaxCoord.z() -= dblSeaLevel;
                }

                osg::Vec3d ptMinPos, ptMaxPos, ptCenterPos;
                pEllipsoidModel->convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), ptMinCoord.z(), ptMinPos.x(), ptMinPos.y(), ptMinPos.z());
                pEllipsoidModel->convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), ptMaxCoord.z(), ptMaxPos.x(), ptMaxPos.y(), ptMaxPos.z());
                pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), ptCenterCoord.z(), ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

                //计算地块的半径
                const double dblRadius1 = (ptMinPos - ptMaxPos).length() * 0.5;
                const double dblRadius2 = (ptCenterPos - ptMinPos).length();
                const double dblRadius3 = (ptCenterPos - ptMaxPos).length();
                const double dblRadius  = std::max(dblRadius1, std::max(dblRadius2, dblRadius3));

                osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
                const ID child_id(0u, nChildLevel, nChildRow, nChildCol, nChildHeight);
                pPagedLOD->setFileID(0u, child_id);
                pPagedLOD->setRange(0u, 0.0, dblRadius * dblCubeRangeRatio);
                pPagedLOD->setCenter(ptCenterPos);
                pPagedLOD->setRadius(dblRadius);

                if(!vecValid[nChildIndex])
                {
                    pPagedLOD->setNodeMask(0);
                }
                ++nChildIndex;
                pGroup->addChild(pPagedLOD.get());
            }
        }
    }

    osg::ref_ptr<VCubeFragmentGroup> pFragmentGroup = new VCubeFragmentGroup;
    pFragmentGroup->create(m_pVCubeManager.get(), id, cubeObjectIDs, m_pLODFixer.get());
    pGroup->addChild(pFragmentGroup.get());

    return pGroup.release();
}


void VirtualCubeReaderWriter::readObjects(const ID &id, const VCubeData &pairVCData, bool vecValid[], ObjectCube &cubeObjectIDs) const
{
    cubeObjectIDs.clear();

    memset(vecValid, 0, 8u);

    const unsigned nFragCount = m_pVCubeManager->getCubeFragmentCount();
    cubeObjectIDs.resize(nFragCount);
    for(unsigned y = 0u; y < nFragCount; y++)
    {
        ObjectMatrix &objectMatrix = cubeObjectIDs[y];
        objectMatrix.resize(nFragCount);
        for(unsigned x = 0u; x < nFragCount; x++)
        {
            ObjectRow &objectRow = objectMatrix[x];
            objectRow.resize(nFragCount);
        }
    }

    OpenSP::sp<vcm::IVirtualCube>   pRemoteVCube = NULL;
    const void *pData = pairVCData.first;
    unsigned int nLen = pairVCData.second;
    if(pData != NULL && nLen > 0u)
    {
        pRemoteVCube = vcm::createVirtualCubeByStream(pData, nLen);
    }

    OpenSP::sp<vcm::IVirtualCube> pVCube = m_pVCubeManager->copyVirtualCube(id);
    if(pVCube)
    {
        if(pRemoteVCube.valid())
        {
            pVCube->mergeVCube(pRemoteVCube.get());
        }
    }
    else
    {
        if(pRemoteVCube.valid())
        {
            pVCube = pRemoteVCube;
        }
    }

    if(!pVCube.valid()) return;

    pVCube->getChildCubeState(vecValid);

    for(unsigned y = 0u; y < nFragCount; y++)
    {
        ObjectMatrix &objectMatrix = cubeObjectIDs[y];
        for(unsigned x = 0u; x < nFragCount; x++)
        {
            ObjectRow &objectRow = objectMatrix[x];
            for(unsigned z = 0u; z < nFragCount; z++)
            {
                IDList &idList = objectRow[z];
                pVCube->getFragmentObjects(y, x, z, idList);
            }
        }
    }
}


