// VirtualTileManager.cpp : 定义 DLL 应用程序的导出函数。
//
#include "VirtualTileManager.h"

#include "EllipsoidModel.h"

#include <OpenSP/sp.h>
#include <Common/Common.h>
#include <assert.h>
#include <sstream>
#include <IDProvider/Definer.h>
#include <DEUDB/IDEUDB.h>
#include <deque>

namespace vtm
{

UINT_64 getRefTime(void)
{
    UINT_64 nUpdateTime = 0u;
    cmm::genUniqueValue64(nUpdateTime);
    return nUpdateTime;
}


IVirtualTileManager *createVirtualTileManager(void)
{
    OpenSP::sp<VirtualTileManager> pVirtualTileManager = new VirtualTileManager();

    if(pVirtualTileManager.valid())
    {
        pVirtualTileManager->initialize();
        return pVirtualTileManager.release();
    }

    return NULL;
}


IVirtualTile *createVirtualTileByStream(const void *pBuffer, unsigned nLength)
{
    OpenSP::sp<VirtualTile> pVTile = new VirtualTile;
    if(!pBuffer || nLength < 1u)
    {
        return NULL;
    }

    bson::bsonDocument  bsonDoc;
    bson::bsonStream    bs;
    bs.Write(pBuffer, nLength);
    bs.Reset();
    bsonDoc.Read(&bs);
    pVTile->fromBson(bsonDoc);

    return pVTile.release();
}


bool VirtualTile::addObject(const ID &id, double dblX, double dblY)
{
    cmm::math::Point2d ptMin, ptMax;
    cmm::Pyramid::instance()->getTilePos(m_nLevel, m_nRow, m_nCol, ptMin.x(), ptMin.y(), ptMax.x(), ptMax.y());

    const cmm::math::Box2d boxArea(ptMin, ptMax);

    const double dblFragmentCount = g_nFragmentCount;
    const double dblIntervalX = boxArea.width() / dblFragmentCount;
    const double dblIntervalY = boxArea.height() / dblFragmentCount;

    const double dblBiasX = dblX - boxArea.left();
    const double dblBiasY = dblY - boxArea.bottom();

    const double dblPosX = dblBiasX / dblIntervalX;
    const double dblPosY = dblBiasY / dblIntervalY;

    const double dblFragCounts = g_nFragmentCount;
    if(dblPosX >= dblFragCounts || dblPosX < 0.0
        || dblPosY >= dblFragCounts || dblPosY < 0.0)
    {
        return false;
    }

    const unsigned nPosX = unsigned(dblPosX);
    const unsigned nPosY = unsigned(dblPosY);

    if(m_mtxFragments[nPosY][nPosX].addObject(id))
    {
        return true;
    }
    return false;
}


bool VirtualTile::removeObject(const ID &id)
{
    for(unsigned y = 0u; y < g_nFragmentCount; y++)
    {
        for(unsigned x = 0u; x < g_nFragmentCount; x++)
        {
            if(m_mtxFragments[y][x].removeObject(id))
            {
                return true;
            }
        }
    }
    return false;
}


bool VirtualTile::toBson(bson::bsonDocument &bsonDoc) const
{
    bson::bsonArrayEle *pBsonValid = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Valid"));

    for(unsigned int i = 0u; i < 4u; i++)
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

            const IDList &listObjects = m_mtxFragments[y][x].m_listObjects;
            IDList::const_iterator itorID = listObjects.begin();
            for( ; itorID != listObjects.end(); ++itorID)
            {
                const ID &id = *itorID;
                pFragmentCol->AddBinElement((void *)&id, sizeof(ID));
            }
        }
    }

    return true;
}


bool VirtualTile::fromBson(const bson::bsonDocument &bsonDoc)
{
    //Valid
    const bson::bsonArrayEle *pBsonValid = dynamic_cast<const bson::bsonArrayEle *>(bsonDoc.GetElement("Valid"));
    if(!pBsonValid)
    {
        assert(false);
        return false;
    }

    for(unsigned int i = 0u; i < 4u; i++)
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

            IDList &listObjects = m_mtxFragments[y][x].m_listObjects;
            listObjects.clear();

            const unsigned nObjCount = pFragmentCol->ChildCount();
            for(unsigned int i = 0u; i < nObjCount; i++)
            {
                const bson::bsonElement *pObject = pFragmentCol->GetElement(i);

                const ID id(pObject->BinData(), pObject->BinDataLen());
                listObjects.push_back(id);
            }
            std::sort(listObjects.begin(), listObjects.end());
        }
    }
    return true;
}


void VirtualTile::mergeVTile(const IVirtualTile *pTile)
{
    const VirtualTile *pTileObj = dynamic_cast<const VirtualTile *>(pTile);
    for(unsigned int i = 0u; i < 4u; i++)
    {
        m_bChildValid[i] = (m_bChildValid[i] || pTileObj->m_bChildValid[i]);
    }

    const unsigned nFragmentCount = g_nFragmentCount;
    for(unsigned int y = 0u; y < nFragmentCount; y++)
    {
        for(unsigned int x = 0u; x < nFragmentCount; x++)
        {
            IDList &listObjects1 = m_mtxFragments[y][x].m_listObjects;
            const IDList &listObjects2 = pTileObj->m_mtxFragments[y][x].m_listObjects;
            listObjects1.insert(listObjects1.end(), listObjects2.begin(), listObjects2.end());
            std::sort(listObjects1.begin(), listObjects1.end());

            IDList::iterator itorEnd = std::unique(listObjects1.begin(), listObjects1.end());
            listObjects1.erase(itorEnd, listObjects1.end());
        }
    }
}


VirtualTileManager::VirtualTileManager(void) :
    m_nMaxLevel(18u),
    m_nMemoryLimited(512u),
    m_bChangingListening(false)
{
}


VirtualTileManager::~VirtualTileManager(void)
{

}


void VirtualTileManager::addTileToWindow(const ID &idTile, const VTileInfo &info)
{
    m_mapBuffer[info.m_nLastRefTime] = idTile;
    m_mapVTilesWindow[idTile] = info;
    limitWindowSize();
    registerChangedVTile(idTile);
}


void VirtualTileManager::limitWindowSize(void)
{
    if(m_mapVTilesWindow.size() <= m_nMemoryLimited)
    {
        return;
    }

    const ID &idUnload = m_mapBuffer.begin()->second;

    std::map<ID, VTileInfo>::const_iterator itorUnload = m_mapVTilesWindow.find(idUnload);
    assert(itorUnload != m_mapVTilesWindow.end());

    const VTileInfo &infoUnload = itorUnload->second;

    bson::bsonDocument bsonDoc;
    infoUnload.m_pVirtualTile->toBson(bsonDoc);
    bson::bsonStream bStream;
    bsonDoc.Write(&bStream);
    bStream.Reset();
    m_pVTileDB->replaceBlock(idUnload, bStream.Data(), bStream.DataLen());

    m_mapBuffer.erase(m_mapBuffer.begin());
    m_mapVTilesWindow.erase(itorUnload);
}


void VirtualTileManager::reLoadTile(const ID &idTile)
{
    if(m_mapVTilesWindow.find(idTile) != m_mapVTilesWindow.end())
    {
        return;
    }

    void *pBuffer = NULL;
    unsigned nLength = 0u;
    const bool bExist = m_pVTileDB->readBlock(idTile, pBuffer, nLength);
    if(!bExist) return;

    VTileInfo info;
    info.m_pVirtualTile = loadTile(pBuffer, nLength);
    deudbProxy::freeMemory(pBuffer);

    info.m_nLastRefTime = getRefTime();
    info.m_pVirtualTile->m_nLevel = idTile.TileID.m_nLevel;
    info.m_pVirtualTile->m_nRow   = idTile.TileID.m_nRow;
    info.m_pVirtualTile->m_nCol   = idTile.TileID.m_nCol;
    addTileToWindow(idTile, info);
}


VirtualTile *VirtualTileManager::loadTile(const void *pBuffer, unsigned nLength) const
{
    bson::bsonStream bStream;
    bStream.Write(pBuffer, nLength);
    bStream.Reset();

    bson::bsonDocument bsonDoc;
    bsonDoc.Read(&bStream);

    VirtualTile *pVTile = new VirtualTile;
    pVTile->fromBson(bsonDoc);
    return pVTile;
}


void VirtualTileManager::removeTile(const ID &idTile)
{
    std::map<ID, VTileInfo>::const_iterator itorFind = m_mapVTilesWindow.find(idTile);
    if(itorFind != m_mapVTilesWindow.end())
    {
        const VTileInfo &info = itorFind->second;
        m_mapBuffer.erase(info.m_nLastRefTime);
        m_mapVTilesWindow.erase(itorFind);
    }
    m_pVTileDB->removeBlock(idTile);
}


bool VirtualTileManager::initialize(void)
{
    const std::string strVTileDB = cmm::genLocalTempDB2();
    const UINT_64   nBufferSize = 1024ui64 * 1024ui64 * 800ui64;
    m_pVTileDB = deudbProxy::createDEUDBProxy();
    if(!m_pVTileDB->openDB(strVTileDB, nBufferSize, nBufferSize))
    {
        return false;
    }
    m_pVTileDB->setClearFlag(true);
    return true;
}


bool VirtualTileManager::save2DB(const std::string &strTargetDB)
{
    std::map<ID, VTileInfo>::const_iterator itor = m_mapVTilesWindow.begin();
    for(; itor != m_mapVTilesWindow.end(); ++itor)
    {
        const ID &idTile = itor->first;
        const VTileInfo &info = itor->second;

        //不存在此TileID，直接写入
        bson::bsonDocument bsonDoc;
        info.m_pVirtualTile->toBson(bsonDoc);

        bson::bsonStream bs;
        bsonDoc.Write(&bs);
        m_pVTileDB->replaceBlock(idTile, bs.Data(), bs.DataLen());
    }

    // 接下来要不要用文件拷贝呢？
    // 还是算了吧！讲究一点职业道德，一个一个的block进行拷贝还是比较靠谱一些，防止DEUDB那边将文件后缀改了就麻烦了

    OpenSP::sp<deudb::IDEUDB>   pTargetDB = deudb::createDEUDB();
    if(!pTargetDB->openDB(strTargetDB))
    {
        return false;
    }

    const std::vector<ID> vecAllIndices = m_pVTileDB->getAllIndices();
    for(std::vector<ID>::const_iterator itorBlock = vecAllIndices.begin(); itorBlock != vecAllIndices.end(); ++itorBlock)
    {
        const ID &id = *itorBlock;

        void *pBuffer = NULL;
        unsigned nLength = 0u;
        if(!m_pVTileDB->readBlock(id, pBuffer, nLength))
        {
            continue;
        }

        pTargetDB->replaceBlock(id, pBuffer, nLength);
        deudb::freeMemory(pBuffer);
    }

    return true;
}


bool VirtualTileManager::getLevelRowCol(double dblX, double dblY, double dblHeight, double dblRadius, unsigned &nLevel, unsigned &nRow, unsigned &nCol) const
{
    nLevel = nRow = nCol = 0u;

    nLevel = m_nMaxLevel;
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    pPyramid->getTile(m_nMaxLevel, dblX, dblY, nRow, nCol);

    const EllipsoidModel em;
    const double dblTileRangeRatio = getTileRangeRatio();
    double dblTileViewRange = 0.0;
    do{
        cmm::math::Point2d ptMinCoord, ptMaxCoord;
        pPyramid->getTilePos(nLevel, nRow, nCol, ptMinCoord.x(), ptMinCoord.y(), ptMaxCoord.x(), ptMaxCoord.y());

        cmm::math::Point3d ptMin, ptMax;
        em.convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), 0.0, ptMin.x(), ptMin.y(), ptMin.z());
        em.convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), 0.0, ptMax.x(), ptMax.y(), ptMax.z());

        double tmpX, tmpY, tmpZ;
        tmpX = ptMax.x() - ptMin.x();
        tmpY = ptMax.y() - ptMin.y();
        tmpZ = ptMax.z() - ptMin.z();
        const double dblTileRadius = sqrt(tmpX * tmpX + tmpY * tmpY + tmpZ * tmpZ) * 0.5;
        dblTileViewRange = dblTileRadius * dblTileRangeRatio;
        if(dblTileViewRange - dblRadius >= dblRadius)
        {
            break;
        }

        const unsigned nLevelParent = nLevel - 1u;
        unsigned nRowParent, nColParent;
        if(!pPyramid->getParentByLevel(nLevel, nRow, nCol, nLevelParent, nRowParent, nColParent))
        {
            break;
        }

        nLevel = nLevelParent;
        nRow = nRowParent;
        nCol = nColParent;
    }while(nLevel > 2u);

    return true;
}


bool VirtualTileManager::addObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius)
{
    unsigned nLevel, nRow, nCol;
    if(!getLevelRowCol(dblX, dblY, dblHeight, dblRadius, nLevel, nRow, nCol))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVTilesWindow);

    ID idTile(0, nLevel, nRow, nCol, 0);
    std::map<ID, VTileInfo>::iterator itorFind = m_mapVTilesWindow.find(idTile);

    if(itorFind == m_mapVTilesWindow.end())
    {
        reLoadTile(idTile);

        itorFind = m_mapVTilesWindow.find(idTile);
    }

    if(itorFind != m_mapVTilesWindow.end())
    {
        VTileInfo &info = itorFind->second;
        m_mapBuffer.erase(info.m_nLastRefTime);
        info.m_nLastRefTime = getRefTime();
        m_mapBuffer[info.m_nLastRefTime] = idTile;
        const bool bAdd = info.m_pVirtualTile->addObject(id, dblX, dblY);
        if(bAdd)
        {
            registerChangedVTile(idTile);
        }
        return bAdd;
    }

    // 不存在此TileID
    VTileInfo    info;
    info.m_pVirtualTile = new VirtualTile;
    info.m_pVirtualTile->m_nLevel = idTile.TileID.m_nLevel;
    info.m_pVirtualTile->m_nRow   = idTile.TileID.m_nRow;
    info.m_pVirtualTile->m_nCol   = idTile.TileID.m_nCol;
    info.m_nLastRefTime = getRefTime();

    if(!info.m_pVirtualTile->addObject(id, dblX, dblY))
    {
        return false;
    }
    registerChangedVTile(idTile);

    addTileToWindow(idTile, info);

    //父节点ID
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    unsigned nLevel_p = 0, nRow_p = 0, nCol_p = 0;
    if(!pPyramid->getParent(nLevel, nRow, nCol, nLevel_p, nRow_p, nCol_p))
    {
        return true;
    }

    ID idParent(0u, nLevel_p, nRow_p, nCol_p, 0);
    while(true)
    {
        std::map<ID, VTileInfo>::iterator itorParent = m_mapVTilesWindow.find(idParent);

        unsigned nIndexInParent = 0u;
        if(idTile.TileID.m_nLevel == 1u)
        {
            nIndexInParent = idTile.TileID.m_nCol % 2u;
        }
        else
        {
            nIndexInParent = (idTile.TileID.m_nRow % 2u) * 2u + idTile.TileID.m_nCol % 2u;
        }

        if(itorParent == m_mapVTilesWindow.end())
        {
            reLoadTile(idParent);
            itorParent = m_mapVTilesWindow.find(idParent);
        }

        if(itorParent != m_mapVTilesWindow.end())
        {
            // 父节点存在
            VTileInfo &infoParent = itorParent->second;
            m_mapBuffer.erase(infoParent.m_nLastRefTime);
            infoParent.m_nLastRefTime = getRefTime();
            m_mapBuffer[infoParent.m_nLastRefTime] = idParent;
            bool &bValidFlag = infoParent.m_pVirtualTile->m_bChildValid[nIndexInParent];
            if(!bValidFlag)
            {
                bValidFlag = true;
                registerChangedVTile(idParent);
            }
            break;
        }


        // 父节点不存在，需要创建出来
        VTileInfo infoParent;
        infoParent.m_pVirtualTile = new VirtualTile;
        infoParent.m_pVirtualTile->m_nLevel = idParent.TileID.m_nLevel;
        infoParent.m_pVirtualTile->m_nRow   = idParent.TileID.m_nRow;
        infoParent.m_pVirtualTile->m_nCol   = idParent.TileID.m_nCol;
        infoParent.m_pVirtualTile->m_bChildValid[nIndexInParent] = true;
        infoParent.m_nLastRefTime = getRefTime();
        addTileToWindow(idParent, infoParent);
        registerChangedVTile(idParent);

        nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u;
        if(!pPyramid->getParent(idParent.TileID.m_nLevel, idParent.TileID.m_nRow, idParent.TileID.m_nCol, nLevel_p, nRow_p, nCol_p))
        {
            break;
        }

        idTile = idParent;
        idParent.set(0u, nLevel_p, nRow_p, nCol_p, 0);
    }

    return true;
}


bool VirtualTileManager::removeObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius)
{
    unsigned nLevel, nRow, nCol;
    if(!getLevelRowCol(dblX, dblY, dblHeight, dblRadius, nLevel, nRow, nCol))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVTilesWindow);

    ID idTile(0, nLevel, nRow, nCol, 0);

    //数据库中无此TileID，直接返回
    std::map<ID, VTileInfo>::iterator itorFind = m_mapVTilesWindow.find(idTile);
    if(itorFind == m_mapVTilesWindow.end())
    {
        reLoadTile(idTile);
        itorFind = m_mapVTilesWindow.find(idTile);
    }

    if(itorFind == m_mapVTilesWindow.end())
    {
        return false;
    }

    VTileInfo &info = itorFind->second;
    m_mapBuffer.erase(info.m_nLastRefTime);
    info.m_nLastRefTime = getRefTime();
    m_mapBuffer[info.m_nLastRefTime] = idTile;

    //在此Tile中查找此id，未找到直接返回
    if(!info.m_pVirtualTile->isExist(id))
    {
        return false;
    }

    if(!info.m_pVirtualTile->removeObject(id))
    {
        return false;
    }

    //若不存在Object且无子节点，此Tile可以被删除
    const unsigned nObjCount = info.m_pVirtualTile->getObjectsCount();
    if(nObjCount > 0u ||
       info.m_pVirtualTile->m_bChildValid[0] ||
       info.m_pVirtualTile->m_bChildValid[1] ||
       info.m_pVirtualTile->m_bChildValid[2] ||
       info.m_pVirtualTile->m_bChildValid[3])
    {
        registerChangedVTile(idTile);
        return true;
    }

    removeTile(idTile);
    registerChangedVTile(idTile);

    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    //父节点ID
    unsigned nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u;
    if(!pPyramid->getParent(nLevel, nRow, nCol, nLevel_p, nRow_p, nCol_p))
    {
        return true;
    }

    //Tile_ID在其父节点中的索引
    ID idParent(0u, nLevel_p, nRow_p, nCol_p, 0);
    while(true)
    {
        std::map<ID, VTileInfo>::iterator itorParent = m_mapVTilesWindow.find(idParent);
        if(itorParent == m_mapVTilesWindow.end())
        {
            reLoadTile(idParent);

            itorParent = m_mapVTilesWindow.find(idTile);
        }
        assert(itorParent != m_mapVTilesWindow.end());

        unsigned nIndexInParent = 0u;
        if(idTile.TileID.m_nLevel == 1u)
        {
            nIndexInParent = idTile.TileID.m_nCol % 2u;
        }
        else
        {
            nIndexInParent = (idTile.TileID.m_nRow % 2u) * 2u + idTile.TileID.m_nCol % 2u;
        }

        VTileInfo &infoParent = itorParent->second;
        m_mapBuffer.erase(infoParent.m_nLastRefTime);
        infoParent.m_nLastRefTime = getRefTime();
        m_mapBuffer[infoParent.m_nLastRefTime] = idParent;

        infoParent.m_pVirtualTile->m_bChildValid[nIndexInParent] = false;

        const unsigned nObjCount = infoParent.m_pVirtualTile->getObjectsCount();
        if(nObjCount > 0u ||
            infoParent.m_pVirtualTile->m_bChildValid[0] ||
            infoParent.m_pVirtualTile->m_bChildValid[1] ||
            infoParent.m_pVirtualTile->m_bChildValid[2] ||
            infoParent.m_pVirtualTile->m_bChildValid[3])
        {
            registerChangedVTile(idParent);
            return true;
        }

        removeTile(idParent);
        registerChangedVTile(idParent);

        // 父节点ID
        unsigned nLevel_p = 0u, nRow_p = 0u, nCol_p = 0u;
        if(!pPyramid->getParent(idParent.TileID.m_nLevel, idParent.TileID.m_nRow, idParent.TileID.m_nCol, nLevel_p, nRow_p, nCol_p))
        {
            break;
        }

        idTile = idParent;
        idParent.set(0u, nLevel_p, nRow_p, nCol_p, 0);
    }
    return true;
}


void VirtualTileManager::getObjectsOnTile(const ID &idTile, IDList &vecObjectIDs, bool valid[]) const
{
    const VirtualTile *pVTile = dynamic_cast<const VirtualTile *>(getVirtualTile(idTile));
    if(!pVTile) return;

    const unsigned nCount = pVTile->getObjectsCount();
    vecObjectIDs.reserve(nCount);
    pVTile->getObjects(vecObjectIDs);

    valid[0] = pVTile->m_bChildValid[0];
    valid[1] = pVTile->m_bChildValid[1];
    valid[2] = pVTile->m_bChildValid[2];
    valid[3] = pVTile->m_bChildValid[3];
}


const IVirtualTile *VirtualTileManager::getVirtualTile(const ID &idTile) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVTilesWindow);

    std::map<ID, VTileInfo>::const_iterator itorFind = m_mapVTilesWindow.find(idTile);
    if(itorFind == m_mapVTilesWindow.end())
    {
        const_cast<VirtualTileManager *>(this)->reLoadTile(idTile);
        itorFind = m_mapVTilesWindow.find(idTile);
    }
    if(itorFind == m_mapVTilesWindow.end())
    {
        return NULL;
    }

    const VTileInfo &info = itorFind->second;
    return info.m_pVirtualTile.get();
}


IVirtualTile *VirtualTileManager::copyVirtualTile(const ID &idTile) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxVTilesWindow);

    std::map<ID, VTileInfo>::const_iterator itorFind = m_mapVTilesWindow.find(idTile);
    if(itorFind == m_mapVTilesWindow.end())
    {
        const_cast<VirtualTileManager *>(this)->reLoadTile(idTile);
        itorFind = m_mapVTilesWindow.find(idTile);
    }
    if(itorFind == m_mapVTilesWindow.end())
    {
        return NULL;
    }

    const VTileInfo &info = itorFind->second;
    VirtualTile *pVTile = new VirtualTile(*info.m_pVirtualTile);
    return pVTile;
}


bool VirtualTileManager::takeChangedVTile(IDList &idList)
{
    if(!m_bChangingListening)
    {
        return false;
    }

    m_blockChangedVTile.block();
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxChangedVTile);
        idList.assign(m_setChangedVTile.begin(), m_setChangedVTile.end());
        m_setChangedVTile.clear();
        m_blockChangedVTile.reset();
    }

    return true;
}


void VirtualTileManager::startChangingListening(void)
{
    m_bChangingListening = true;
    m_blockChangedVTile.release();
}


void VirtualTileManager::stopChangingListening(void)
{
    m_bChangingListening = false;
    m_blockChangedVTile.release();
}


void VirtualTileManager::registerChangedVTile(const ID &id)
{
    if(!m_bChangingListening)
    {
        return;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxChangedVTile);
    m_setChangedVTile.insert(id);
    m_blockChangedVTile.release();
}


}

