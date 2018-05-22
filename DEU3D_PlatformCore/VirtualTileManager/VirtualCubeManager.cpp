// VirtualCubeManager.cpp : 定义 DLL 应用程序的导出函数。
//
#include "VirtualCubeManager.h"

#include "EllipsoidModel.h"

#include <OpenSP/sp.h>
#include <Common/Common.h>
#include <assert.h>
#include <sstream>
#include <IDProvider/Definer.h>
#include <DEUDB/IDEUDB.h>
#include <deque>

namespace vcm
{

UINT_64 getRefTime(void)
{
    UINT_64 nUpdateTime = 0u;
    cmm::genUniqueValue64(nUpdateTime);
    return nUpdateTime;
}


IVirtualCubeManager *createVirtualCubeManager(void)
{
    OpenSP::sp<VirtualCubeManager> pVirtualCubeManager = new VirtualCubeManager();

    if(pVirtualCubeManager.valid())
    {
        pVirtualCubeManager->initialize();
        return pVirtualCubeManager.release();
    }

    return NULL;
}


IVirtualCube *createVirtualCubeByStream(const void *pBuffer, unsigned nLength)
{
    OpenSP::sp<VirtualCube> pVCube = new VirtualCube;
    if(!pBuffer || nLength < 1u)
    {
        return NULL;
    }

    bson::bsonDocument  bsonDoc;
    bsonDoc.FromBsonStream(pBuffer, nLength);
    pVCube->fromBson(bsonDoc);

    return pVCube.release();
}


bool VirtualCube::addObject(const ID &id, const cmm::math::Point3d &pos)
{
    const EllipsoidModel em;
    const double dblSeaLevel = em.computeLocalSeaLevel(pos.y(), pos.x());
    const cmm::math::Point3d position(pos.x(), pos.y(), pos.z() + dblSeaLevel);

    cmm::math::Point3d ptMin, ptMax;
    cmm::Pyramid3::instance()->getCubePos(m_nLevel, m_nRow, m_nCol, m_nHeight, ptMin, ptMax);

    const cmm::math::Box3d boxArea(ptMin, ptMax);

    const double dblFragmentCount = g_nFragmentCount;
    const double dblIntervalX = boxArea.width() / dblFragmentCount;
    const double dblIntervalY = boxArea.height() / dblFragmentCount;
    const double dblIntervalZ = boxArea.depth() / dblFragmentCount;

    const cmm::math::Point3d ptBoxCorner = boxArea.corner(cmm::math::Box3d::NearLeftBottom);
    const double dblBiasX = position.x() - ptBoxCorner.x();
    const double dblBiasY = position.y() - ptBoxCorner.y();
    const double dblBiasZ = position.z() - ptBoxCorner.z();

    const double dblPosX = dblBiasX / dblIntervalX;
    const double dblPosY = dblBiasY / dblIntervalY;
    const double dblPosZ = dblBiasZ / dblIntervalZ;

    const double dblFragCounts = g_nFragmentCount;
    if(dblPosX >= dblFragCounts || dblPosX < 0.0
        || dblPosY >= dblFragCounts || dblPosY < 0.0
        || dblPosZ >= dblFragCounts || dblPosZ < 0.0)
    {
        return false;
    }

    const unsigned nPosX = unsigned(dblPosX);
    const unsigned nPosY = unsigned(dblPosY);
    const unsigned nPosZ = unsigned(dblPosZ);

    if(m_mtxFragments[nPosY][nPosX][nPosZ].addObject(id))
    {
        return true;
    }
    return false;
}


bool VirtualCube::removeObject(const ID &id)
{
    for(unsigned y = 0u; y < g_nFragmentCount; y++)
    {
        for(unsigned x = 0u; x < g_nFragmentCount; x++)
        {
            for(unsigned z = 0u; z < g_nFragmentCount; z++)
            {
                if(m_mtxFragments[y][x][z].removeObject(id))
                {
                    return true;
                }
            }
        }
    }
    return false;
}


bool VirtualCube::toBson(bson::bsonDocument &bsonDoc) const
{
    bson::bsonArrayEle *pBsonValid = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Valid"));

    for(unsigned int i = 0u; i < 8u; i++)
    {
        pBsonValid->AddBoolElement(m_bChildValid[i]);
    }

    const unsigned nFragmentCount = g_nFragmentCount;
    bson::bsonArrayEle *pFragmentMatrix = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("FragmentMatrix"));
    for(unsigned y = 0u; y < nFragmentCount; y++)
    {
        bson::bsonArrayEle *pFragmentRow = dynamic_cast<bson::bsonArrayEle *>(pFragmentMatrix->AddArrayElement());
        for(unsigned x = 0u; x < nFragmentCount; x++)
        {
            bson::bsonArrayEle *pFragmentCol = dynamic_cast<bson::bsonArrayEle *>(pFragmentRow->AddArrayElement());

            for(unsigned z = 0u; z < nFragmentCount; z++)
            {
                bson::bsonArrayEle *pFragmentHeight = dynamic_cast<bson::bsonArrayEle *>(pFragmentCol->AddArrayElement());

                const IDList &listObjects = m_mtxFragments[y][x][z].m_listObjects;
                IDList::const_iterator itorID = listObjects.begin();
                for( ; itorID != listObjects.end(); ++itorID)
                {
                    const ID &id = *itorID;
                    pFragmentHeight->AddBinElement((void *)&id, sizeof(ID));
                }
            }
        }
    }

    return true;
}


bool VirtualCube::fromBson(const bson::bsonDocument &bsonDoc)
{
    //Valid
    const bson::bsonArrayEle *pBsonValid = dynamic_cast<const bson::bsonArrayEle *>(bsonDoc.GetElement("Valid"));
    if(!pBsonValid)
    {
        assert(false);
        return false;
    }

    for(unsigned int i = 0u; i < 8u; i++)
    {
        const bson::bsonElement *pBsonEle = pBsonValid->GetElement(i);
        if(!pBsonEle)
        {
            assert(false);
            return false;
        }
        m_bChildValid[i] = pBsonEle->BoolValue();
    }

    const unsigned nFragmentCount = g_nFragmentCount;
    const bson::bsonArrayEle *pFragmentMatrix = dynamic_cast<const bson::bsonArrayEle *>(bsonDoc.GetElement("FragmentMatrix"));
    for(unsigned int y = 0u; y < nFragmentCount; y++)
    {
        const bson::bsonArrayEle *pFragmentRow = dynamic_cast<const bson::bsonArrayEle *>(pFragmentMatrix->GetElement(y));
        for(unsigned int x = 0u; x < nFragmentCount; x++)
        {
            const bson::bsonArrayEle *pFragmentCol = dynamic_cast<const bson::bsonArrayEle *>(pFragmentRow->GetElement(x));

            for(unsigned int z = 0u; z < nFragmentCount; z++)
            {
                const bson::bsonArrayEle *pFragmentHeight = dynamic_cast<const bson::bsonArrayEle *>(pFragmentCol->GetElement(z));

                IDList &listObjects = m_mtxFragments[y][x][z].m_listObjects;
                listObjects.clear();

                const unsigned nObjCount = pFragmentHeight->ChildCount();
                for(unsigned int i = 0u; i < nObjCount; i++)
                {
                    const bson::bsonElement *pObject = pFragmentHeight->GetElement(i);

                    const ID id(pObject->BinData(), pObject->BinDataLen());
                    listObjects.push_back(id);
                }
                std::sort(listObjects.begin(), listObjects.end());
            }
        }
    }
    return true;
}


void VirtualCube::mergeVCube(const IVirtualCube *pCube)
{
    const VirtualCube *pCubeObj = dynamic_cast<const VirtualCube *>(pCube);
    for(unsigned int i = 0u; i < 8u; i++)
    {
        m_bChildValid[i] = (m_bChildValid[i] || pCubeObj->m_bChildValid[i]);
    }

    const unsigned nFragmentCount = g_nFragmentCount;
    for(unsigned int y = 0u; y < nFragmentCount; y++)
    {
        for(unsigned int x = 0u; x < nFragmentCount; x++)
        {
            for(unsigned int z = 0u; z < nFragmentCount; z++)
            {
                IDList &listObjects1 = m_mtxFragments[y][x][z].m_listObjects;
                const IDList &listObjects2 = pCubeObj->m_mtxFragments[y][x][z].m_listObjects;
                listObjects1.insert(listObjects1.end(), listObjects2.begin(), listObjects2.end());
                std::sort(listObjects1.begin(), listObjects1.end());

                IDList::iterator itorEnd = std::unique(listObjects1.begin(), listObjects1.end());
                listObjects1.erase(itorEnd, listObjects1.end());
            }
        }
    }
}


VirtualCubeManager::VirtualCubeManager(void) :
    m_nMaxLevel(18u),
    m_nMemoryLimited(512u),
    m_bChangingListening(false)
{
}


VirtualCubeManager::~VirtualCubeManager(void)
{

}


void VirtualCubeManager::addCubeToWindow(const ID &idCube, const VCubeInfo &info)
{
    m_mapBuffer[info.m_nLastRefTime] = idCube;
    m_mapVCubesWindow[idCube] = info;
    limitWindowSize();
    registerChangedVCube(idCube);
}


void VirtualCubeManager::limitWindowSize(void)
{
    if(m_mapVCubesWindow.size() <= m_nMemoryLimited)
    {
        return;
    }

    const ID &idUnload = m_mapBuffer.begin()->second;

    std::map<ID, VCubeInfo>::const_iterator itorUnload = m_mapVCubesWindow.find(idUnload);
    assert(itorUnload != m_mapVCubesWindow.end());

    const VCubeInfo &infoUnload = itorUnload->second;

    bson::bsonDocument bsonDoc;
    infoUnload.m_pVirtualCube->toBson(bsonDoc);
    bson::bsonStream bStream;
    bsonDoc.Write(&bStream);
    bStream.Reset();
    m_pVCubeDB->replaceBlock(idUnload, bStream.Data(), bStream.DataLen());

    m_mapBuffer.erase(m_mapBuffer.begin());
    m_mapVCubesWindow.erase(itorUnload);
}


void VirtualCubeManager::reLoadCube(const ID &idCube)
{
    if(m_mapVCubesWindow.find(idCube) != m_mapVCubesWindow.end())
    {
        return;
    }

    void *pBuffer = NULL;
    unsigned nLength = 0u;
    const bool bExist = m_pVCubeDB->readBlock(idCube, pBuffer, nLength);
    if(!bExist) return;

    VCubeInfo info;
    info.m_pVirtualCube = loadCube(pBuffer, nLength);
    deudbProxy::freeMemory(pBuffer);

    info.m_nLastRefTime = getRefTime();
    info.m_pVirtualCube->m_nLevel  = idCube.CubeID.m_nLevel;
    info.m_pVirtualCube->m_nRow    = idCube.CubeID.m_nRow;
    info.m_pVirtualCube->m_nCol    = idCube.CubeID.m_nCol;
    info.m_pVirtualCube->m_nHeight = idCube.CubeID.m_nHeight;
    addCubeToWindow(idCube, info);
}


VirtualCube *VirtualCubeManager::loadCube(const void *pBuffer, unsigned nLength) const
{
    bson::bsonDocument bsonDoc;
    bsonDoc.FromBsonStream(pBuffer, nLength);

    VirtualCube *pVCube = new VirtualCube;
    pVCube->fromBson(bsonDoc);
    return pVCube;
}


void VirtualCubeManager::removeCube(const ID &idTile)
{
    std::map<ID, VCubeInfo>::const_iterator itorFind = m_mapVCubesWindow.find(idTile);
    if(itorFind != m_mapVCubesWindow.end())
    {
        const VCubeInfo &info = itorFind->second;
        m_mapBuffer.erase(info.m_nLastRefTime);
        m_mapVCubesWindow.erase(itorFind);
    }
    m_pVCubeDB->removeBlock(idTile);
}


bool VirtualCubeManager::initialize(void)
{
    const std::string strVCubeDB = cmm::genLocalTempDB2();
    const UINT_64   nBufferSize = 1024ui64 * 1024ui64 * 800ui64;
    m_pVCubeDB = deudbProxy::createDEUDBProxy();
    if(!m_pVCubeDB->openDB(strVCubeDB, nBufferSize, nBufferSize))
    {
        return false;
    }
    m_pVCubeDB->setClearFlag(true);
    return true;
}


bool VirtualCubeManager::save2DB(const std::string &strTargetDB)
{
    std::map<ID, VCubeInfo>::const_iterator itor = m_mapVCubesWindow.begin();
    for(; itor != m_mapVCubesWindow.end(); ++itor)
    {
        const ID &idCube = itor->first;
        const VCubeInfo &info = itor->second;

        //不存在此CubeID，直接写入
        bson::bsonDocument bsonDoc;
        info.m_pVirtualCube->toBson(bsonDoc);

        bson::bsonStream bs;
        bsonDoc.Write(&bs);
        m_pVCubeDB->replaceBlock(idCube, bs.Data(), bs.DataLen());
    }

    // 接下来要不要用文件拷贝呢？
    // 还是算了吧！讲究一点职业道德，一个一个的block进行拷贝还是比较靠谱一些，防止DEUDB那边将文件后缀改了就麻烦了

    OpenSP::sp<deudb::IDEUDB>   pTargetDB = deudb::createDEUDB();
    if(!pTargetDB->openDB(strTargetDB))
    {
        return false;
    }

    const std::vector<ID> vecAllIndices = m_pVCubeDB->getAllIndices();
    for(std::vector<ID>::const_iterator itorBlock = vecAllIndices.begin(); itorBlock != vecAllIndices.end(); ++itorBlock)
    {
        const ID &id = *itorBlock;

        void *pBuffer = NULL;
        unsigned nLength = 0u;
        if(!m_pVCubeDB->readBlock(id, pBuffer, nLength))
        {
            continue;
        }

        pTargetDB->replaceBlock(id, pBuffer, nLength);
        deudb::freeMemory(pBuffer);
    }

    return true;
}


bool VirtualCubeManager::getLevelRowColHeight(const cmm::math::Sphered &sphere, unsigned &nLevel, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const
{
    nLevel = nRow = nCol = nHeight = 0u;

    nLevel = m_nMaxLevel;
    const cmm::Pyramid3 *pPyramid = cmm::Pyramid3::instance();
    cmm::math::Point3d ptCenter = sphere.getCenter();

    const EllipsoidModel em;
    const double dblSphereSeaLevel = em.computeLocalSeaLevel(ptCenter.y(), ptCenter.x());
    ptCenter.z() += dblSphereSeaLevel;
    pPyramid->getCube(m_nMaxLevel, ptCenter, nRow, nCol, nHeight);

    const double dblCubeRangeRatio = getCubeRangeRatio();
    double dblCubeViewRange = 0.0;
    do{
        cmm::math::Point3d ptMinCoord, ptMaxCoord;
        pPyramid->getCubePos(nLevel, nRow, nCol, nHeight, ptMinCoord, ptMaxCoord);

        const double dblMinPosSeaLevel = em.computeLocalSeaLevel(ptMinCoord.y(), ptMinCoord.x());
        const double dblMaxPosSeaLevel = em.computeLocalSeaLevel(ptMaxCoord.y(), ptMaxCoord.x());

        cmm::math::Point3d ptMin, ptMax;
        em.convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), ptMinCoord.z() - dblMinPosSeaLevel, ptMin.x(), ptMin.y(), ptMin.z());
        em.convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), ptMinCoord.z() - dblMaxPosSeaLevel, ptMax.x(), ptMax.y(), ptMax.z());

        const cmm::math::Vector3d vecDiagonal = ptMax - ptMin;
        const double dblCubeRadius = vecDiagonal.length() * 0.5;
        dblCubeViewRange = dblCubeRadius * dblCubeRangeRatio;
        if(dblCubeViewRange - sphere.getRadius() >= sphere.getRadius())
        {
            break;
        }

        const unsigned nLevelParent = nLevel - 1u;
        unsigned nRowParent, nColParent, nHeightParent;
        if(!pPyramid->getParentByLevel(nLevel, nRow, nCol, nHeight, nLevelParent, nRowParent, nColParent, nHeightParent))
        {
            break;
        }

        nLevel = nLevelParent;
        nRow = nRowParent;
        nCol = nColParent;
        nHeight = nHeightParent;
    }while(nLevel > 2u);

    return true;
}


bool VirtualCubeManager::addObject(const ID &id, const cmm::math::Sphered &sphere)
{
    unsigned nLevel, nRow, nCol, nHeight;
    if(!getLevelRowColHeight(sphere, nLevel, nRow, nCol, nHeight))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVCubesWindow);

    ID idCube(0, nLevel, nRow, nCol, nHeight);
    std::map<ID, VCubeInfo>::iterator itorFind = m_mapVCubesWindow.find(idCube);

    if(itorFind == m_mapVCubesWindow.end())
    {
        reLoadCube(idCube);

        itorFind = m_mapVCubesWindow.find(idCube);
    }

    if(itorFind != m_mapVCubesWindow.end())
    {
        VCubeInfo &info = itorFind->second;
        m_mapBuffer.erase(info.m_nLastRefTime);
        info.m_nLastRefTime = getRefTime();
        m_mapBuffer[info.m_nLastRefTime] = idCube;
        const bool bAdd = info.m_pVirtualCube->addObject(id, sphere.getCenter());
        if(bAdd)
        {
            registerChangedVCube(idCube);
        }
        return bAdd;
    }

    // 不存在此CubeID
    VCubeInfo    info;
    info.m_pVirtualCube = new VirtualCube;
    info.m_pVirtualCube->m_nLevel = idCube.CubeID.m_nLevel;
    info.m_pVirtualCube->m_nRow   = idCube.CubeID.m_nRow;
    info.m_pVirtualCube->m_nCol   = idCube.CubeID.m_nCol;
    info.m_pVirtualCube->m_nHeight = idCube.CubeID.m_nHeight;
    info.m_nLastRefTime = getRefTime();

    if(!info.m_pVirtualCube->addObject(id, sphere.getCenter()))
    {
        return false;
    }
    registerChangedVCube(idCube);

    addCubeToWindow(idCube, info);

    //父节点ID
    const cmm::Pyramid3 *pPyramid = cmm::Pyramid3::instance();
    unsigned nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u, nHeight_p = 0u;
    if(!pPyramid->getParent(nLevel, nRow, nCol, nHeight, nLevel_p, nRow_p, nCol_p, nHeight_p))
    {
        return true;
    }

    ID idParent(0u, nLevel_p, nRow_p, nCol_p, nHeight_p);
    while(true)
    {
        std::map<ID, VCubeInfo>::iterator itorParent = m_mapVCubesWindow.find(idParent);

        unsigned nIndexInParent = 0u;
        if(idCube.CubeID.m_nLevel == 1u)
        {
            nIndexInParent = (idCube.CubeID.m_nCol % 2u) * 2u + idCube.CubeID.m_nHeight % 2u;
        }
        else
        {
            nIndexInParent = (idCube.CubeID.m_nRow % 2u) * 2u * 2u + (idCube.CubeID.m_nCol % 2u) * 2u + idCube.CubeID.m_nHeight % 2u;
        }

        if(itorParent == m_mapVCubesWindow.end())
        {
            reLoadCube(idParent);
            itorParent = m_mapVCubesWindow.find(idParent);
        }

        if(itorParent != m_mapVCubesWindow.end())
        {
            // 父节点存在
            VCubeInfo &infoParent = itorParent->second;
            m_mapBuffer.erase(infoParent.m_nLastRefTime);
            infoParent.m_nLastRefTime = getRefTime();
            m_mapBuffer[infoParent.m_nLastRefTime] = idParent;
            bool &bValidFlag = infoParent.m_pVirtualCube->m_bChildValid[nIndexInParent];
            if(!bValidFlag)
            {
                bValidFlag = true;
                registerChangedVCube(idParent);
            }
            break;
        }

        // 父节点不存在，需要创建出来
        VCubeInfo infoParent;
        infoParent.m_pVirtualCube = new VirtualCube;
        infoParent.m_pVirtualCube->m_nLevel  = idParent.CubeID.m_nLevel;
        infoParent.m_pVirtualCube->m_nRow    = idParent.CubeID.m_nRow;
        infoParent.m_pVirtualCube->m_nCol    = idParent.CubeID.m_nCol;
        infoParent.m_pVirtualCube->m_nHeight = idParent.CubeID.m_nHeight;
        infoParent.m_pVirtualCube->m_bChildValid[nIndexInParent] = true;
        infoParent.m_nLastRefTime = getRefTime();
        addCubeToWindow(idParent, infoParent);
        registerChangedVCube(idParent);

        nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u, nHeight_p = 0;
        if(!pPyramid->getParent(idParent.CubeID.m_nLevel, idParent.CubeID.m_nRow, idParent.CubeID.m_nCol, idParent.CubeID.m_nHeight, nLevel_p, nRow_p, nCol_p, nHeight_p))
        {
            break;
        }

        idCube = idParent;
        idParent.set(0u, nLevel_p, nRow_p, nCol_p, nHeight_p);
    }

    return true;
}


bool VirtualCubeManager::removeObject(const ID &id, const cmm::math::Sphered &sphere)
{
    unsigned nLevel, nRow, nCol, nHeight;
    if(!getLevelRowColHeight(sphere, nLevel, nRow, nCol, nHeight))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVCubesWindow);

    ID idCube(0, nLevel, nRow, nCol, nHeight);

    //数据库中无此TileID，直接返回
    std::map<ID, VCubeInfo>::iterator itorFind = m_mapVCubesWindow.find(idCube);
    if(itorFind == m_mapVCubesWindow.end())
    {
        reLoadCube(idCube);
        itorFind = m_mapVCubesWindow.find(idCube);
    }

    if(itorFind == m_mapVCubesWindow.end())
    {
        return false;
    }

    VCubeInfo &info = itorFind->second;
    m_mapBuffer.erase(info.m_nLastRefTime);
    info.m_nLastRefTime = getRefTime();
    m_mapBuffer[info.m_nLastRefTime] = idCube;

    //在此Tile中查找此id，未找到直接返回
    if(!info.m_pVirtualCube->isExist(id))
    {
        return false;
    }

    if(!info.m_pVirtualCube->removeObject(id))
    {
        return false;
    }

    //若不存在Object且无子节点，此Tile可以被删除
    const unsigned nObjCount = info.m_pVirtualCube->getObjectsCount();
    if(nObjCount > 0u ||
       info.m_pVirtualCube->m_bChildValid[0] ||
       info.m_pVirtualCube->m_bChildValid[1] ||
       info.m_pVirtualCube->m_bChildValid[2] ||
       info.m_pVirtualCube->m_bChildValid[3] ||
       info.m_pVirtualCube->m_bChildValid[4] ||
       info.m_pVirtualCube->m_bChildValid[5] ||
       info.m_pVirtualCube->m_bChildValid[6] ||
       info.m_pVirtualCube->m_bChildValid[7])
    {
        registerChangedVCube(idCube);
        return true;
    }

    removeCube(idCube);
    registerChangedVCube(idCube);

    const cmm::Pyramid3 *pPyramid = cmm::Pyramid3::instance();
    //父节点ID
    unsigned nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u, nHeight_p = 0u;
    if(!pPyramid->getParent(nLevel, nRow, nCol, nHeight, nLevel_p, nRow_p, nCol_p, nHeight_p))
    {
        return true;
    }

    //Tile_ID在其父节点中的索引
    ID idParent(0u, nLevel_p, nRow_p, nCol_p, nHeight_p);
    while(true)
    {
        std::map<ID, VCubeInfo>::iterator itorParent = m_mapVCubesWindow.find(idParent);
        if(itorParent == m_mapVCubesWindow.end())
        {
            reLoadCube(idParent);

            itorParent = m_mapVCubesWindow.find(idParent);
        }
        assert(itorParent != m_mapVCubesWindow.end());

        unsigned nIndexInParent = 0u;
        if(idCube.CubeID.m_nLevel == 1u)
        {
            nIndexInParent = (idCube.CubeID.m_nCol % 2u) * 2u + idCube.CubeID.m_nHeight % 2u;
        }
        else
        {
            nIndexInParent = (idCube.CubeID.m_nRow % 2u) * 2u * 2u + (idCube.CubeID.m_nCol % 2u) * 2u + idCube.CubeID.m_nHeight % 2u;
        }

        VCubeInfo &infoParent = itorParent->second;
        m_mapBuffer.erase(infoParent.m_nLastRefTime);
        infoParent.m_nLastRefTime = getRefTime();
        m_mapBuffer[infoParent.m_nLastRefTime] = idParent;

        infoParent.m_pVirtualCube->m_bChildValid[nIndexInParent] = false;

        const unsigned nObjCount = infoParent.m_pVirtualCube->getObjectsCount();
        if(nObjCount > 0u ||
            infoParent.m_pVirtualCube->m_bChildValid[0] ||
            infoParent.m_pVirtualCube->m_bChildValid[1] ||
            infoParent.m_pVirtualCube->m_bChildValid[2] ||
            infoParent.m_pVirtualCube->m_bChildValid[3] ||
            infoParent.m_pVirtualCube->m_bChildValid[4] ||
            infoParent.m_pVirtualCube->m_bChildValid[5] ||
            infoParent.m_pVirtualCube->m_bChildValid[6] ||
            infoParent.m_pVirtualCube->m_bChildValid[7])
        {
            registerChangedVCube(idParent);
            return true;
        }

        removeCube(idParent);
        registerChangedVCube(idParent);

        // 父节点ID
        if(!pPyramid->getParent(idParent.CubeID.m_nLevel, idParent.CubeID.m_nRow, idParent.CubeID.m_nCol, idParent.CubeID.m_nHeight, nLevel_p, nRow_p, nCol_p, nHeight_p))
        {
            break;
        }

        idCube = idParent;
        idParent.set(0u, nLevel_p, nRow_p, nCol_p, nHeight_p);
    }
    return true;
}


void VirtualCubeManager::getObjectsOnCube(const ID &idCube, IDList &vecObjectIDs, bool valid[]) const
{
    const VirtualCube *pVCube = dynamic_cast<const VirtualCube *>(getVirtualCube(idCube));
    if(!pVCube) return;

    const unsigned nCount = pVCube->getObjectsCount();
    vecObjectIDs.reserve(nCount);
    pVCube->getObjects(vecObjectIDs);

    memcpy(valid, pVCube->m_bChildValid, sizeof(pVCube->m_bChildValid));
}


const IVirtualCube *VirtualCubeManager::getVirtualCube(const ID &idCube) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVCubesWindow);

    std::map<ID, VCubeInfo>::const_iterator itorFind = m_mapVCubesWindow.find(idCube);
    if(itorFind == m_mapVCubesWindow.end())
    {
        const_cast<VirtualCubeManager *>(this)->reLoadCube(idCube);
        itorFind = m_mapVCubesWindow.find(idCube);
    }
    if(itorFind == m_mapVCubesWindow.end())
    {
        return NULL;
    }

    const VCubeInfo &info = itorFind->second;
    return info.m_pVirtualCube.get();
}


IVirtualCube *VirtualCubeManager::copyVirtualCube(const ID &idCube) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVCubesWindow);

    std::map<ID, VCubeInfo>::const_iterator itorFind = m_mapVCubesWindow.find(idCube);
    if(itorFind == m_mapVCubesWindow.end())
    {
        const_cast<VirtualCubeManager *>(this)->reLoadCube(idCube);
        itorFind = m_mapVCubesWindow.find(idCube);
    }
    if(itorFind == m_mapVCubesWindow.end())
    {
        return NULL;
    }

    const VCubeInfo &info = itorFind->second;
    VirtualCube *pVCube = new VirtualCube(*info.m_pVirtualCube);
    return pVCube;
}


bool VirtualCubeManager::takeChangedVCube(IDList &idList)
{
    if(!m_bChangingListening)
    {
        return false;
    }

    m_blockChangedVCube.block();
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxChangedVCube);
        idList.assign(m_setChangedVCube.begin(), m_setChangedVCube.end());
        m_setChangedVCube.clear();
        m_blockChangedVCube.reset();
    }

    return true;
}


void VirtualCubeManager::startChangingListening(void)
{
    m_bChangingListening = true;
    m_blockChangedVCube.release();
}


void VirtualCubeManager::stopChangingListening(void)
{
    m_bChangingListening = false;
    m_blockChangedVCube.release();
}


void VirtualCubeManager::registerChangedVCube(const ID &id)
{
    if(!m_bChangingListening)
    {
        return;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxChangedVCube);
    m_setChangedVCube.insert(id);
    m_blockChangedVCube.release();
}


}

