#include "FileReadInterceptor.h"

#include <osgDB/Registry>
#include <osgDB/Options>
#include <osgDB/FileNameUtils>
#include <osg/ValueObject>
#include <osgDB/WriteFile>
#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Optimizer>
#include <osg/Timer>
#include <osg/PagedLOD>
#include <osg/SharedObjectPool>

#include <istream>
#include <strstream>
#include <stdlib.h>
#include <time.h>

#include <Common/Common.h>
#include <Common/deuImage.h>
#include <EventAdapter/EventAdapter.h>
#include <EventAdapter/IEventFilter.h>
#include <EventAdapter/IEventObject.h>

#include <ParameterSys/Parameter.h>
#include <ParameterSys/Detail.h>
#include <IDProvider/Definer.h>

#include "VirtualTileReaderWriter.h"
#include "TileObjBlender.h"
#include "StateBase.h"
#include "../LogicalManager/ILayerManager.h"
#include <ExternalService/IWMTSDriver.h>
#include "Registry.h"

#define DEBUG_LOG 1

osg::Matrix computeTexMat(const ID &SrcID, const ID &DesID)
{
    const cmm::Pyramid *pyr = cmm::Pyramid::instance();

    //计算源ID的范围
    double dblSrcMinX, dblSrcMinY, dblSrcMaxX, dblSrcMaxY;
    pyr->getTilePos(SrcID.TileID.m_nLevel, SrcID.TileID.m_nRow, SrcID.TileID.m_nCol, dblSrcMinX, dblSrcMinY, dblSrcMaxX, dblSrcMaxY);

    //计算目标的范围
    double dblDesMinX, dblDesMinY, dblDesMaxX, dblDesMaxY;
    pyr->getTilePos(DesID.TileID.m_nLevel, DesID.TileID.m_nRow, DesID.TileID.m_nCol, dblDesMinX, dblDesMinY, dblDesMaxX, dblDesMaxY);

    double dblDesCenterX = (dblDesMaxX + dblDesMinX) / 2.0;
    double dblDesCenterY = (dblDesMaxY + dblDesMinY) / 2.0;
    double dblDesWidth = dblDesMaxX - dblDesMinX;
    double dblDesHeight = dblDesMaxY - dblDesMinY;

    int nPosX = (dblDesCenterX - dblSrcMinX) / dblDesWidth;
    int nPosY = (dblDesCenterY - dblSrcMinY) / dblDesHeight;

    double nRadio = 1.0 / powf(2, DesID.TileID.m_nLevel - SrcID.TileID.m_nLevel);

    osg::Matrix mtx;
    mtx.postMultScale(osg::Vec3(nRadio, nRadio, 0.0));
    mtx.postMultTranslate(osg::Vec3(nPosX * nRadio, nPosY * nRadio, 0.0));

    return mtx;
}


class TextureEraser : public osg::NodeVisitor
{
public:
    enum TextureOp
    {
        TO_NO_OP,
        TO_ERASE_WHILE_LOAD,
        TO_ERASE_WHILE_COMPILE
    };

    TextureOp   m_eTextureOp;
    explicit TextureEraser(TextureOp eOp) :
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        m_eTextureOp(eOp)
    {
    }

    void eraseTextureWhileCompile(osg::StateSet *pStateSet)
    {
        for(unsigned int i = 0; i < pStateSet->getNumTextureAttributeLists(); ++i)
        {
            osg::Texture2D *pTexture = dynamic_cast<osg::Texture2D *>(pStateSet->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            if(!pTexture)   continue;
            const osg::Image *pImage = pTexture->getImage();
            if(!pImage)     continue;

            if(pImage->getID().ObjectID.m_nType != SHARE_IMAGE_ID)
            {
                pTexture->setUnRefImageDataAfterApply(true);
            }
        }
    }
    void eraseTextureWhileLoad(osg::StateSet *pStateSet)
    {
        for(unsigned int i = 0; i < pStateSet->getNumTextureAttributeLists(); ++i)
        {
            osg::Texture2D *pTexture = dynamic_cast<osg::Texture2D *>(pStateSet->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            if(!pTexture)   continue;

            pStateSet->removeTextureAttribute(i, pTexture);
        }
    }
    virtual void apply(osg::Node& node)
    {
        osg::StateSet *pStateSet = node.getStateSet();
        if(!pStateSet)
        {
            osg::NodeVisitor::apply(node);
            return;
        }

        if(m_eTextureOp == TO_ERASE_WHILE_LOAD)
        {
            eraseTextureWhileLoad(pStateSet);
        }
        else if(m_eTextureOp == TO_ERASE_WHILE_COMPILE)
        {
            eraseTextureWhileCompile(pStateSet);
        }
        osg::NodeVisitor::apply(node);
    }
    virtual void apply(osg::Geode& node)
    {
        const unsigned nCount = node.getNumDrawables();
        for(unsigned n = 0; n < nCount; n++)
        {
            osg::Drawable *pDrawable = node.getDrawable(n);
            if(!pDrawable)  continue;

            pDrawable->setUseDisplayList(true);

            osg::StateSet *pStateSet = pDrawable->getStateSet();
            if(!pStateSet)  continue;

            if(m_eTextureOp == TO_ERASE_WHILE_LOAD)
            {
                eraseTextureWhileLoad(pStateSet);
            }
            else if(m_eTextureOp == TO_ERASE_WHILE_COMPILE)
            {
                eraseTextureWhileCompile(pStateSet);
            }
        }

        osg::StateSet *pStateSet = node.getStateSet();
        if(!pStateSet)
        {
            osg::NodeVisitor::apply(node);
            return;
        }

        if(m_eTextureOp == TO_ERASE_WHILE_LOAD)
        {
            eraseTextureWhileLoad(pStateSet);
        }
        else if(m_eTextureOp == TO_ERASE_WHILE_COMPILE)
        {
            eraseTextureWhileCompile(pStateSet);
        }

        osg::NodeVisitor::apply(node);
    }
};


class MemoryEraser
{
public:
    explicit MemoryEraser(void) : m_bFromLocal(true), m_pMemBuffer(NULL){}
    explicit MemoryEraser(bool bFromLocal, void *pMemBuffer) : m_bFromLocal(bFromLocal), m_pMemBuffer(pMemBuffer){}
    virtual ~MemoryEraser(void)
    {
        if(m_pMemBuffer)
        {
            if(m_bFromLocal)
            {
                deudbProxy::freeMemory(m_pMemBuffer);
            }
            else
            {
                deunw::freeMemory(m_pMemBuffer);
            }
        }
    }

public:
    bool    m_bFromLocal;
    void   *m_pMemBuffer;
};

class DisplayListApplier : public osg::NodeVisitor
{
public:
    explicit DisplayListApplier(bool bValid) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_bUseDisplayList(bValid){}
    virtual ~DisplayListApplier(void){}

    virtual void apply(osg::Geode& node)
    {
        const unsigned nCount = node.getNumDrawables();
        for(unsigned n = 0; n < nCount; n++)
        {
            osg::Drawable *pDrawable = node.getDrawable(n);
            if(!pDrawable)  continue;

            pDrawable->setUseDisplayList(m_bUseDisplayList);
            pDrawable->setUseVertexBufferObjects(!m_bUseDisplayList);
        }

        osg::NodeVisitor::apply(node);
    }

protected:
    bool    m_bUseDisplayList;
};


FileReadInterceptor::FileReadInterceptor(void)
    : m_pDEUNetwork(NULL),
    m_pStateManager(NULL)
{
}


FileReadInterceptor::~FileReadInterceptor(void)
{
    waitForRequestFinish();

    if(m_pLocalTempDB.valid())
    {
        m_pLocalTempDB->closeDB();
        m_pLocalTempDB = NULL;
    }

    LocalDBMap::iterator itorDataSet = m_mapLocalDBs.begin();
    for( ; itorDataSet != m_mapLocalDBs.end(); ++itorDataSet)
    {
        std::vector<LocalDB> &vecDB = itorDataSet->second;
        std::vector<LocalDB>::iterator itorDB = vecDB.begin();
        for( ; itorDB != vecDB.end(); ++itorDB)
        {
            LocalDB &db = *itorDB;
            if(db.m_pDeuDB.valid())
            {
                db.m_pDeuDB->closeDB();
            }
        }
    }
    m_pDEUNetwork = NULL;

    m_pSharedObjectPool = NULL;
}


void FileReadInterceptor::waitForRequestFinish(void)
{
    while((unsigned)m_MissionStatus > 0u)
    {
        OpenThreads::Thread::YieldCurrentThread();
        OpenThreads::Thread::microSleep(1000);
    }
}


bool FileReadInterceptor::login(const std::string& strAuthHost, const std::string& strAuthPort, const std::string& strUserName, const std::string& strUserPwd)
{
    if (m_pDEUNetwork == NULL)
    {
        m_pDEUNetwork = deunw::createDEUNetwork();
    }

    return m_pDEUNetwork->login(strAuthHost, strAuthPort, strUserName, strUserPwd, NULL);
}


bool FileReadInterceptor::logout()
{
    if (m_pDEUNetwork != NULL)
    {
        return m_pDEUNetwork->logout();
    }

    return false;
}


bool FileReadInterceptor::initialize(bool bBifurcateThread,
    const std::string &strHost,
    const std::string &strPort,
    const std::string &strLocalCache,
    cmm::IStateQuerier *pStateQuerier)
{
    if (m_pDEUNetwork == NULL)
    {
        m_pDEUNetwork = deunw::createDEUNetwork();
    }

    if(!m_pDEUNetwork->initialize(strHost, strPort, true, strLocalCache))
    {
        std::cout << "Warning: cannot find the remote server, force network component to initialize." << std::endl;
    }

    const std::string strLocalTempDB = cmm::genLocalTempDB();
    m_pLocalTempDB = deudbProxy::createDEUDBProxy();
    if(m_pLocalTempDB->openDB(strLocalTempDB))
    {
        m_pLocalTempDB->setClearFlag(true);
    }
    else
    {
        m_pLocalTempDB = NULL;
    }

    osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
    pPool->initialize();

    m_bBifurcateThread = bBifurcateThread;
    m_pStateQuerier    = pStateQuerier;

    logical::ILayerManager *pLayerManager = dynamic_cast<logical::ILayerManager *>(pStateQuerier);
    vcm::IVirtualCubeManager *pVCubeManager = pLayerManager->getVirtualCubeManager();

    m_pVirtualCubeReaderWriter  = new VirtualCubeReaderWriter(pVCubeManager);
    const std::string strIVE = "ive";
    const std::string strJPG = "jpg";
    const std::string strPNG = "png";
    const std::string strBMP = "bmp";
    const std::string strTIF = "tif";
    const std::string strDDS = "dds";
    m_pIveReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strIVE);
    m_pJpgReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strJPG);
    m_pPngReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strPNG);
    m_pBmpReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strBMP);
    m_pTifReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strTIF);
    m_pDdsReaderWriter = osgDB::Registry::instance()->getReaderWriterForExtension(strDDS);

    m_pTextCenterLayouter = new TextCenterLayouter;

    m_pSharedObjectPool = new SharedObjectPool;

    return true;
}

bool FileReadInterceptor::addLocalDatabase(const std::string &strDB)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxLocalDBs);
    if(!m_mapLocalDBs.empty())
    {
        LocalDBMap::const_iterator itorFind = std::find_if(m_mapLocalDBs.begin(), m_mapLocalDBs.end(), LocalDBFinder(strDB));
        if(itorFind != m_mapLocalDBs.end())
        {
            return false;
        }
    }

    OpenSP::sp<deudbProxy::IDEUDBProxy> pDeuDB = deudbProxy::createDEUDBProxy();
    if(!pDeuDB->openDB(strDB))
    {
        return false;
    }

    // this code segment will be removed
    const std::vector<ID> vecIDs = pDeuDB->getAllIndices();
    if(vecIDs.empty())  return false;

    std::set<unsigned>  setDatasetCodes;
    for(std::vector<ID>::const_iterator itorID = vecIDs.begin(); itorID != vecIDs.end(); ++itorID)
    {
        const ID &id = *itorID;

        if(id.m_nHighBit == 0 && id.m_nLowBit == 0 && id.m_nMidBit == 0)
        {
            continue;
        }
        setDatasetCodes.insert(id.ObjectID.m_nDataSetCode);
    }

    std::string strDB_2Upper = strDB;
    std::transform(strDB_2Upper.begin(), strDB_2Upper.end(), strDB_2Upper.begin(), ::toupper);

    for(std::set<unsigned>::const_iterator itorCode = setDatasetCodes.begin(); itorCode != setDatasetCodes.end(); ++itorCode)
    {
        const unsigned &nDatasetCode = *itorCode;

        std::vector<LocalDB> &vecDBs = m_mapLocalDBs[nDatasetCode];
        std::vector<LocalDB>::const_iterator itorFind = std::find_if(vecDBs.begin(), vecDBs.end(), LocalDBFinder(strDB_2Upper));
        if(itorFind == vecDBs.end())
        {
            vecDBs.push_back(LocalDB(pDeuDB, strDB_2Upper));
        }
    }

    return true;
}


bool FileReadInterceptor::removeLocalDatabase(const std::string &strDB)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxLocalDBs);

    LocalDBMap::iterator itorFind = std::find_if(m_mapLocalDBs.begin(), m_mapLocalDBs.end(), LocalDBFinder(strDB));
    if(itorFind == m_mapLocalDBs.end())
    {
        return false;
    }

    m_mapLocalDBs.erase(itorFind);
    return true;
}


void FileReadInterceptor::setTerrainLayersOrder(bool bDEM, const IDList &vecTerrainOrder)
{
    std::map<unsigned __int64, TerrainTilesInfo>  mapTerrainTopCovers;
    std::vector<TerrainOrderItem>   vecItems;
    vecItems.reserve(vecTerrainOrder.size());
    for(IDList::const_iterator itor = vecTerrainOrder.begin(); itor != vecTerrainOrder.end(); ++itor)
    {
        const ID &id = *itor;
        if(!id.isValid())   continue;

        if(bDEM)
        {
            if(id.ObjectID.m_nType != TERRAIN_DEM_ID)   continue;
        }
        else
        {
            if(id.ObjectID.m_nType != TERRAIN_DOM_ID)   continue;
        }

        TerrainTilesInfo terrainInfo;

        std::map<ID, OpenSP::sp<deues::ITileSet> >::iterator itorWMTS = m_mapWMTSTileSet.find(id);
        if(itorWMTS != m_mapWMTSTileSet.end())
        {
            OpenSP::sp<deues::ITileSet> pTileSet = itorWMTS->second;
            void *pBuffer = NULL;
            unsigned nLength = 0u;
            if(!pTileSet->getTopInfo(pBuffer, nLength))
            {
                continue;
            }
            if(!pBuffer || nLength == 0u)
            {
                if(pBuffer) deues::freeMemory(pBuffer);
                continue;
            }

            parseTerrainInfo(pBuffer, nLength, terrainInfo);
        }
        else
        {
            if(!fetchTerrainInfo(id, terrainInfo))
            {
                continue;
            }
        }

        if(terrainInfo.empty())
        {
            continue;
        }

        TerrainOrderItem item;
        item.m_id = id;
        item.m_LayerItem.m_nUniqueID = terrainInfo.begin()->first.TileID.m_nUniqueID;
        item.m_LayerItem.m_nDatasetCode = terrainInfo.begin()->first.TileID.m_nDataSetCode;
        vecItems.push_back(item);

        mapTerrainTopCovers[item.m_LayerItem.m_nUniqueID].swap(terrainInfo);
    }

    if(bDEM)
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDemCoverOrder);
            m_vecDemCoverOrder.swap(vecItems);
        }
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDemTopTiles);
            m_mapDemTopTiles.swap(mapTerrainTopCovers);
        }
    }
    else
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDomCoverOrder);
            m_vecDomCoverOrder.swap(vecItems);
        }
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDomTopTiles);
            m_mapDomTopTiles.swap(mapTerrainTopCovers);
        }
    }
}


void FileReadInterceptor::getTerrainLayersOrder(bool bDEM, IDList &vecTerrainOrder) const
{
    if(bDEM)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDemCoverOrder);
        for(std::vector<TerrainOrderItem>::const_iterator itor = m_vecDemCoverOrder.begin();
            itor != m_vecDemCoverOrder.end(); ++itor)
        {
            vecTerrainOrder.push_back(itor->m_id);
        }
    }
    else
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDomCoverOrder);
        for(std::vector<TerrainOrderItem>::const_iterator itor = m_vecDomCoverOrder.begin();
            itor != m_vecDomCoverOrder.end(); ++itor)
        {
            vecTerrainOrder.push_back(itor->m_id);
        }
    }
}


bool FileReadInterceptor::fetchTerrainInfo(const ID &idTerrain, TerrainTilesInfo &terrainInfo) const
{
    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = readFromLocalDB(idTerrain, pBuffer, nLength);
    if(!bFromLocal && m_pDEUNetwork.valid())
    {
        if(!m_pDEUNetwork->queryData(idTerrain, pBuffer, nLength))
        {
            return false;
        }
    }

    MemoryEraser eraser(bFromLocal, pBuffer);
    if(!pBuffer || nLength < 1u)
    {
        return false;
    }

    return parseTerrainInfo(pBuffer, nLength, terrainInfo);
}


bool FileReadInterceptor::parseTerrainInfo(const void *pBsonData, unsigned nLength, TerrainTilesInfo &terrainInfo) const
{
    bson::bsonDocument bsonDoc;
    const bool bConv2Bson = bsonDoc.FromBsonStream(pBsonData, nLength);
    if(!bConv2Bson) return false;

    const bson::bsonArrayEle *pChildrenElement = dynamic_cast<const bson::bsonArrayEle *>(bsonDoc.GetElement("ChildrenID"));
    if(!pChildrenElement)
    {
        return false;
    }

    const unsigned nCount = pChildrenElement->ChildCount();
    for(unsigned n = 0u; n < nCount; n++)
    {
        const bson::bsonDocumentEle *pElement = dynamic_cast<const bson::bsonDocumentEle *>(pChildrenElement->GetElement(n));
        if(!pElement)   continue;

        const bson::bsonDocument &bsonDocItem = pElement->GetDoc();
        const bson::bsonInt32Ele *pItemEle = dynamic_cast<const bson::bsonInt32Ele *>(bsonDocItem.GetElement(0u));
        if(!pItemEle)   continue;

        const std::string strTitle = pItemEle->EName();
        const ID id = ID::genIDfromString(strTitle);
        if(!id.isValid())   continue;

        const unsigned nMaxLevel = pItemEle->Int32Value();

        terrainInfo[id] = nMaxLevel;
    }
    return true;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readImage(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    MissionIncreaser Increaser(m_MissionStatus);

    const std::string strExtension = osgDB::getFileExtension(strFileName);
    if(!strExtension.empty())
    {
        rr = osgDB::Registry::instance()->readImageImplementation(strFileName, pOptions);
        return rr;
    }

    const ID id = ID::genIDfromString(strFileName);
    if(id.isValid())
    {
        return readImage(id, pOptions, pCreationInfo);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readImage(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    MissionIncreaser Increaser(m_MissionStatus);

    if(!id.isValid())   return rr;

    if(id.TileID.m_nType == IMAGE_ID)
    {
        rr = readImageByID(id, pOptions);
        return rr;
    }
    //先在缓存中查找，若未找到需要做缓存处理
    else if(id.TileID.m_nType == SHARE_IMAGE_ID || id.TileID.m_nType == TERRAIN_TILE_HEIGHT_FIELD || id.TileID.m_nType == TERRAIN_TILE_IMAGE)
    {
        osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
        osg::ref_ptr<osg::Object>    pImage;
        if(pPool->findObject(id, pImage))
        {
            rr = pImage.get();
        }
        else
        {
            rr = readImageByID(id, pOptions);
            if(rr.success())
            {
                //共享纹理需要立即加入缓存中
                if(id.TileID.m_nType == SHARE_IMAGE_ID)
                {
                    pPool->addObject(id, rr.getImage());
                }
            }
        }
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readImageByID(const ID &id, const osgDB::Options *pOptions) const
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = readFromLocalDB(id, pBuffer, nLength);
    if(!bFromLocal && m_pDEUNetwork.valid())
    {
        if(!m_pDEUNetwork->queryData(id, pBuffer, nLength))
        {
            return rr;
        }
    }

    MemoryEraser eraser(bFromLocal, pBuffer);
    osg::ref_ptr<osg::Image> pImage = parseImageFromStream(pBuffer, nLength, pOptions);
    if(!pImage.valid())
    {
        return rr;
    }
    pImage->setID(id);
    rr = osgDB::ReaderWriter::ReadResult(pImage.get());

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readNode(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

    MissionIncreaser Increaser(m_MissionStatus);

    // 获取文件的后缀名
    const std::string strExtension = osgDB::getFileExtension(strFileName);
    if(!strExtension.empty())
    {
        rr = osgDB::Registry::instance()->readNodeImplementation(strFileName, pOptions);
        return rr;
    }

    const ID id = ID::genIDfromString(strFileName);
    if(id.isValid())
    {
        return readNode(id, pOptions, pCreationInfo);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readNode(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

    MissionIncreaser Increaser(m_MissionStatus);

    if(!id.isValid())   return rr;

    //非瓦片ID，即为模型ID
    if(id.TileID.m_nType == TERRAIN_TILE)
    {
        rr = readTerrainTileByID(id, pOptions);
    }
    else if(id.TileID.m_nType == VIRTUAL_CUBE)
    {
        rr = readVirtualCubeByID(id, pOptions);
    }
    else if(id.ObjectID.m_nType == MODEL_ID ||
        id.ObjectID.m_nType == SHARE_MODEL_ID)
    {
        rr = readModelByID(id, pOptions);
    }
    else if(id.ObjectID.m_nType == PARAM_POINT_ID ||
        id.ObjectID.m_nType == PARAM_LINE_ID ||
        id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        rr = readParamByID(id, pOptions);
    }
    else if(id.ObjectID.m_nType == DETAIL_PIPE_CONNECTOR_ID ||
        id.ObjectID.m_nType == DETAIL_CUBE_ID ||
        id.ObjectID.m_nType == DETAIL_CYLINDER_ID ||
        id.ObjectID.m_nType == DETAIL_PRISM_ID ||
        id.ObjectID.m_nType == DETAIL_PYRAMID_ID ||
        id.ObjectID.m_nType == DETAIL_SPHERE_ID ||
        id.ObjectID.m_nType == DETAIL_SECTOR_ID ||
        id.ObjectID.m_nType == DETAIL_STATIC_MODEL_ID ||
        id.ObjectID.m_nType == DETAIL_DYN_POINT_ID ||
        id.ObjectID.m_nType == DETAIL_DYN_LINE_ID ||
        id.ObjectID.m_nType == DETAIL_DYN_FACE_ID ||
        id.ObjectID.m_nType == DETAIL_DYN_IMAGE_ID ||
        id.ObjectID.m_nType == DETAIL_BUBBLE_TEXT_ID ||
        id.ObjectID.m_nType == DETAIL_POLYGON_ID ||
        id.ObjectID.m_nType == DETAIL_ROUND_TABLE_ID ||
        id.ObjectID.m_nType == DETAIL_RECT_PIPE_CONNECTOR_ID ||
        id.ObjectID.m_nType == DETAIL_POLYGON_PIPE_CONNECTOR_ID ||
		id.ObjectID.m_nType == DETAIL_DYN_POINT_CLOUD_ID)
    {
        rr = readDetailByID(id, pOptions, pCreationInfo);
    }
    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readModelByID(const ID &id, const osgDB::Options *pOptions) const
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);

    //若是共享模型，先在Cache中查找
    if(id.TileID.m_nType == SHARE_MODEL_ID)
    {
        osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
        osg::ref_ptr<osg::Object>   pObject;
        if(pPool->findObject(id, pObject))
        {
            rr = pObject.get();
            return rr;
        }
    }

    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = readFromLocalDB(id, pBuffer, nLength);
    if(!bFromLocal && m_pDEUNetwork.valid())
    {
        if(!m_pDEUNetwork->queryData(id, pBuffer, nLength))
        {
            return rr;
        }
    }

    MemoryEraser eraser(bFromLocal, pBuffer);
    if(pBuffer && nLength > 0u)
    {
        const std::string strIVE = "ive";
        if(m_pIveReaderWriter.valid())
        {
            std::strstream ss((char *)pBuffer, nLength);
            rr = m_pIveReaderWriter->readNode(ss, pOptions);
        }
    }

    if(!rr.validNode())
    {
        return rr;
    }

    if(id.ObjectID.m_nType == PARAM_LINE_ID || id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        osg::Node *pNode = rr.getNode();

        //pNode->setUpdateCallback(m_pTextCenterLayouter.get());
        //pNode->setEventCallback(m_pTextCenterLayouter.get());
    }
    else if(id.ObjectID.m_nType == SHARE_MODEL_ID)
    {
        osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
        pPool->addObject(id, rr.getNode());
    }

    if(id.ObjectID.m_nType != SHARE_MODEL_ID)
    {
        const char *ptr = ::getenv("DEU_USED_FOR_ATI");
        bool bUseDisplayList = true;
        if(ptr)
        {
            const int nVal = atoi(ptr);
            if(nVal == 1)
            {
                bUseDisplayList = false;
            }
        }

        osg::Node *pNode = rr.getNode();
        DisplayListApplier dla(bUseDisplayList);
        pNode->accept(dla);
    }

    const char *ptr = ::getenv("DEU_REMOVE_MODEL_TEXTURE");
    if(ptr)
    {
        TextureEraser::TextureOp    eTextureOp = TextureEraser::TO_NO_OP;
        const int nVal = atoi(ptr);
        if(nVal == 1)
        {
            eTextureOp = TextureEraser::TO_ERASE_WHILE_LOAD;
        }
        else if(nVal == 2)
        {
            eTextureOp = TextureEraser::TO_ERASE_WHILE_COMPILE;
        }

        if(eTextureOp != TextureEraser::TO_NO_OP)
        {
            osg::Node *pNode = rr.getNode();
            TextureEraser te(eTextureOp);
            pNode->accept(te);
        }
    }

    //osg::ref_ptr<osgUtil::Simplifier> pSimplifier = new osgUtil::Simplifier;
    //rr.getNode()->accept(*pSimplifier);

    return rr;
}


vcm::IVirtualCube *FileReadInterceptor::readRemoteVirtualCubeByID(const ID &id) const
{
    void *pBuffer = NULL;
    unsigned nLength = 0u;

    if(!m_pDEUNetwork.valid())
    {
        return NULL;
    }

    m_pDEUNetwork->queryData(id, pBuffer, nLength);
    if(!pBuffer || nLength < 1u)
    {
        return NULL;
    }

    MemoryEraser eraser(false, pBuffer);
    vcm::IVirtualCube *pVCube = vcm::createVirtualCubeByStream(pBuffer, nLength);
    return pVCube;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readVirtualCubeByID(const ID &id, const osgDB::Options *pOptions) const
{
    void *pBuffer = NULL;
    unsigned nLength = 0u;

    if(m_pDEUNetwork.valid())
    {
        m_pDEUNetwork->queryData(id, pBuffer, nLength);
    }

    MemoryEraser eraser(false, pBuffer);

    osgDB::ReaderWriter::ReadResult rr = m_pVirtualCubeReaderWriter->readNode(id, std::make_pair(pBuffer, nLength), pOptions);

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readTerrainTileByID(const ID &id, const osgDB::Options *pOptions) const
{
    //OpenThreads::Thread::microSleep(1000 * 1000);
    osg::ref_ptr<osg::Group> pGroup = new osg::Group();
    ID idCur(id);
    idCur.TileID.m_nLevel++;

    const unsigned nRowCount = (id.TileID.m_nLevel == 0 ? 1 : 2);
    const unsigned nColCount = 2u;

    if(m_bBifurcateThread)
    {
        std::vector<OpenSP::sp<FetchThread> >    vecFetchThreads;
        idCur.TileID.m_nRow = (id.TileID.m_nRow << 1);
        for(unsigned y = 0u; y < nRowCount; y++)
        {
            idCur.TileID.m_nCol = (id.TileID.m_nCol << 1);
            for(unsigned x = 0u; x < nColCount; x++)
            {
                FetchThread *pThread = new FetchThread(this, &FileReadInterceptor::readActualTileByID);
                vecFetchThreads.push_back(pThread);
                pThread->setID(idCur);
                pThread->setOptions(pOptions);
                pThread->startThread();
                idCur.TileID.m_nCol++;
            }
            idCur.TileID.m_nRow++;
        }

        std::vector<OpenSP::sp<FetchThread> >::iterator itorThread = vecFetchThreads.begin();
        for( ; itorThread != vecFetchThreads.end(); ++itorThread)
        {
            FetchThread *pThread = itorThread->get();
            pThread->join();

            osgDB::ReaderWriter::ReadResult &rr = pThread->getFetchResult();

            osg::Node *pReadNode = rr.getNode();
            if(!pReadNode)  continue;

            osg::Node *pModifiedNode = NULL;
            osg::LOD *pReadResult1 = dynamic_cast<osg::LOD *>(pReadNode);
            if(pReadResult1 && pReadResult1->getNumChildren() > 0u)
            {
                pModifiedNode = pReadResult1->getChild(0);
            }

            if(!pModifiedNode)
            {
                osgTerrain::TerrainTile *pReadResult2 = dynamic_cast<osgTerrain::TerrainTile *>(pReadNode);
                pModifiedNode = pReadResult2;
            }

            if(pModifiedNode && m_pTerrainModificationManager.valid())
            {
                m_pTerrainModificationManager->modifyTerrainTile(pModifiedNode);
            }

            pGroup->addChild(pReadNode);
        }
    }
    else
    {
        idCur.TileID.m_nRow = (id.TileID.m_nRow << 1);
        for(unsigned y = 0u; y < nRowCount; y++)
        {
            idCur.TileID.m_nCol = (id.TileID.m_nCol << 1);
            for(unsigned x = 0u; x < nColCount; x++)
            {
                osgDB::ReaderWriter::ReadResult rr = readActualTileByID(idCur, pOptions);
                osg::Node *pReadNode = rr.getNode();
                if(!pReadNode)
                {
                    idCur.TileID.m_nCol++;
                    continue;
                }

                pGroup->addChild(pReadNode);

                osg::Node *pModifiedNode = NULL;
                osg::LOD *pReadResult1 = dynamic_cast<osg::LOD *>(pReadNode);
                if(pReadResult1 && pReadResult1->getNumChildren() > 0u)
                {
                    pModifiedNode = pReadResult1->getChild(0);
                }

                if(!pModifiedNode)
                {
                    osgTerrain::TerrainTile *pReadResult2 = dynamic_cast<osgTerrain::TerrainTile *>(pReadNode);
                    pModifiedNode = pReadResult2;
                }

                if(pModifiedNode && m_pTerrainModificationManager.valid())
                {
                    m_pTerrainModificationManager->modifyTerrainTile(pModifiedNode);
                }

                idCur.TileID.m_nCol++;
            }
            idCur.TileID.m_nRow++;
        }
    }

    if(pGroup->getNumChildren() == nRowCount * nColCount)
    {
        //pGroup->setID(id);

        //if((dblmin_X == -90.0 || dblmax_Y == 90.0) && id.TileID.m_nLevel > 4)
        //{
        //    //pGroup->removeChildren(0, pGroup->getNumChildren());
        //    for(unsigned int i = 0; i < pGroup->getNumChildren(); i++)
        //    {
        //        osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
        //        if(pPagedLOD == NULL)
        //        {
        //            continue;
        //        }
        //        pPagedLOD->removeChild(1, 1);
        //    }
        //}
        m_TerrainUpdate.exchange(clock());

        return osgDB::ReaderWriter::ReadResult(pGroup.get());
    }

    return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readActualTileByID(const ID &id, const osgDB::Options *pOptions) const
{
    osgDB::ReaderWriter::ReadResult rr = readSimpleTileByID(id, pOptions);
    if(!rr.success())
    {
        return rr;
    }

    osg::ref_ptr<osgTerrain::TerrainTile> pTerrainTile = dynamic_cast<osgTerrain::TerrainTile *>(rr.getNode());
    if(!pTerrainTile.valid())
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }

    osg::ref_ptr<osg::Node> pNode = buildTerrainNode(id, pTerrainTile.get());
    return osgDB::ReaderWriter::ReadResult(pNode);
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readSimpleTileByID(const ID &id, const osgDB::Options *pOptions) const
{
    if(id.TileID.m_nType != TERRAIN_TILE)
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }

    ID idImage  = id;
    ID idHeight = id;

    idImage.TileID.m_nType  = TERRAIN_TILE_IMAGE;
    idHeight.TileID.m_nType = TERRAIN_TILE_HEIGHT_FIELD;
#if 0
    osgDB::ReaderWriter::ReadResult rrImage, rrHeight;
    if(m_bBifurcateThread)
    {
        sp::sm_ptr<FetchThread> pImageThread  = new FetchThread(this, &FileReadInterceptor::readDOMTileLayerByID);
        sp::sm_ptr<FetchThread> pHeightThread = new FetchThread(this, &FileReadInterceptor::readDEMTileLayerByID);

        pImageThread->setID(idImage);
        pHeightThread->setID(idHeight);

        pImageThread->setOptions(pOptions);
        pHeightThread->setOptions(pOptions);

        pImageThread->startThread();
        pHeightThread->startThread();

        pImageThread->join();
        pHeightThread->join();

        rrImage = pImageThread->getFetchResult();
        rrHeight = pHeightThread->getFetchResult();
    }
    else
    {
        rrImage = readDOMTileLayerByID(idImage, pOptions);
        rrHeight = readDEMTileLayerByID(idHeight, pOptions);
    }

    // 合并rrImage和rrHeight:
    osg::ref_ptr<osgTerrain::TerrainTile>   pTerrainTile = buildTerrainTile(id, rrImage.getImage(), rrHeight.getImage());
    if(pTerrainTile.valid())
    {
        return osgDB::ReaderWriter::ReadResult(pTerrainTile);
    }
#else
    osg::ref_ptr<osg::Image> pDemImage = readDEMTileLayerByID(idHeight, pOptions).getImage();

    bool bUseShadow = Registry::instance()->getUseShadow();
    osg::ref_ptr<osgTerrain::TerrainTile> pTerrainTile;
//     if(bUseShadow)
//     {
//         osg::ref_ptr<osg::Texture2D> pDomTexture = readDomImage(idImage);
//         pTerrainTile = buildTerrainTile(id, pDomTexture.get(), pDemImage.get());
//     }
//     else
//     {
        std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > > vecTexture;
        readDom(idImage, vecTexture, pOptions);

        pTerrainTile = buildTerrainTile(id, vecTexture, pDemImage.get());
//    }
    if(pTerrainTile.valid())
    {
        return osgDB::ReaderWriter::ReadResult(pTerrainTile);
    }
#endif
    return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
}


osg::Texture2D *FileReadInterceptor::readDomImage(const ID &id) const
{
    if(id.TileID.m_nType != TERRAIN_TILE_IMAGE)
    {
        return NULL;
    }

    std::vector<LayerItem> vecLayersOrder;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDomCoverOrder);
        if(m_vecDomCoverOrder.empty())
        {
            return NULL;
        }
        vecLayersOrder.resize(m_vecDomCoverOrder.size());
        std::transform(m_vecDomCoverOrder.begin(), m_vecDomCoverOrder.end(), vecLayersOrder.begin(), TerrainItemFlagFetcher());
    }

    std::vector<std::pair<ID, bool> > vecNearestID;
    //vecNearestID.resize(nCount);
    for(unsigned int i = 0; i < vecLayersOrder.size(); i++)
    {
        ID nearest_id;
        bool bIsBottomTile = false;
        ID temp_id = id;
        temp_id.TileID.m_nDataSetCode = vecLayersOrder[i].m_nDatasetCode;
        temp_id.TileID.m_nUniqueID    = vecLayersOrder[i].m_nUniqueID;
        findNearestIDbyID(temp_id, bIsBottomTile, nearest_id);

        if(!nearest_id.isValid())
        {
            continue;
        }
        vecNearestID.push_back(std::make_pair(nearest_id, bIsBottomTile));
    }

    osg::ref_ptr<osg::Image> pResultImage;
    bool bHasAlpha = true;

    for(std::vector<std::pair<ID, bool> >::iterator itor = vecNearestID.begin(); itor != vecNearestID.end(); ++itor)
    {
        osg::ref_ptr<osg::Image> pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(itor->first, NULL, NULL).getImage();

        if(!pTempImage.valid())
        {
            continue;
        }

        //裁剪读取到的瓦片到当前瓦片的范围
        pTempImage = floodImage(itor->first, id, pTempImage);

        if(pResultImage.valid())
        {
            pResultImage = combineImage(pResultImage.get(), pTempImage.get(), true);
        }
        else
        {
            pResultImage = pTempImage;
        }

        if(!doesImageHaveAlpha(pResultImage.get()))
        {
            bHasAlpha = false;
            break;
        }
    }

    if(!pResultImage.valid())
    {
        return NULL;
    }

    osg::ref_ptr<osg::Texture2D> pTexture2D = new osg::Texture2D;
    pTexture2D->setMaxAnisotropy(16.0f);
    pTexture2D->setResizeNonPowerOfTwoHint(false);

    pTexture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    pTexture2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

    pTexture2D->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    pTexture2D->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    pTexture2D->setUnRefImageDataAfterApply(true);

    //if(bHasAlpha)
    //{
    //    clearAlphaAsColor(pResultImage, 255, 255, 255);
    //}

    //当影像有透明通道的时候，需要给Texture加一个UserData作为标记
    switch(pResultImage->getPixelSizeInBits())
    {
    case 32:
        {
            pResultImage = osg::RGBA8888_2_RGBA5551(pResultImage);
            break;
        }
    case 24:
        {
            pResultImage = osg::RGB888_2_RGB565(pResultImage);
            break;
        }
    default:
        return NULL;
    }

    pTexture2D->setImage(pResultImage);

    return pTexture2D.release();
}

void FileReadInterceptor::readDom(const ID &id, std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > > &vecTexture, const osgDB::Options *pOptions) const
{
    if(id.TileID.m_nType != TERRAIN_TILE_IMAGE)
    {
        return;
    }

    std::vector<LayerItem> vecLayersOrder;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDomCoverOrder);
        if(m_vecDomCoverOrder.empty())
        {
            return;
        }
        vecLayersOrder.resize(m_vecDomCoverOrder.size());
        std::transform(m_vecDomCoverOrder.begin(), m_vecDomCoverOrder.end(), vecLayersOrder.begin(), TerrainItemFlagFetcher());
    }

    std::vector<std::pair<ID, bool> > vecNearestID;
    //vecNearestID.resize(nCount);
    for(unsigned int i = 0; i < vecLayersOrder.size(); i++)
    {
        ID nearest_id;
        bool bIsBottomTile = false;
        ID temp_id = id;
        temp_id.TileID.m_nDataSetCode = vecLayersOrder[i].m_nDatasetCode;
        temp_id.TileID.m_nUniqueID    = vecLayersOrder[i].m_nUniqueID;
        findNearestIDbyID(temp_id, bIsBottomTile, nearest_id);

        if(!nearest_id.isValid())
        {
            continue;
        }
        vecNearestID.push_back(std::make_pair(nearest_id, bIsBottomTile));
    }

    for(std::vector<std::pair<ID, bool> >::iterator itor = vecNearestID.begin(); itor != vecNearestID.end(); ++itor)
    {
        //osg::ref_ptr<osg:Texture2D> pTexture;
        osg::ref_ptr<osg::Object> pObject;

        //若是影像的底层瓦片，先从共享池中去找
        if(itor->second)
        {
            m_pSharedObjectPool->findObject(itor->first, pObject);
            osg::ref_ptr<osg::Texture2D> pTexture2D = dynamic_cast<osg::Texture2D *>(pObject.get());

            //查找到，直接使用
            if(pTexture2D.valid())
            {
                osg::ref_ptr<osg::TexMat> pTexMat;

                //只比较行不同即可
                if(id.TileID.m_nLevel != itor->first.TileID.m_nLevel)
                {
                    osg::Matrix mtx = computeTexMat(itor->first, id);
                    pTexMat = new osg::TexMat;
                    pTexMat->setMatrix(mtx);
                }
                vecTexture.push_back(std::make_pair(pTexture2D, pTexMat));

                //UserData为空，表示没有透明通道，可以直接返回。
                if(pTexture2D->getUserData() == NULL)
                {
                    return;
                }
                else
                {
                    continue;
                }
            }
        }

        osg::ref_ptr<osg::Image> pImage = const_cast<FileReadInterceptor *>(this)->readImage(itor->first, pOptions, NULL).getImage();

        if(!pImage.valid()) continue;

        osg::ref_ptr<osg::Texture2D> pTexture2D = new osg::Texture2D;
        pTexture2D->setMaxAnisotropy(16.0f);
        pTexture2D->setResizeNonPowerOfTwoHint(false);

        pTexture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTexture2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

        pTexture2D->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
        pTexture2D->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
        pTexture2D->setUnRefImageDataAfterApply(true);

        //当影像有透明通道的时候，需要给Texture加一个UserData作为标记
        switch(pImage->getPixelSizeInBits())
        {
        case 32:
            {
                pImage = osg::RGBA8888_2_RGBA5551(pImage);
                osg::ref_ptr<osg::Referenced> pReferenced = new osg::Referenced;
                pTexture2D->setUserData(pReferenced);
                break;
            }
        case 24:
            {
                pImage = osg::RGB888_2_RGB565(pImage);
                break;
            }
        default:
            return;
        }

        pTexture2D->setImage(pImage);

        osg::ref_ptr<osg::TexMat> pTexMat;

        //只比较行不同即可
        if(id.TileID.m_nLevel != itor->first.TileID.m_nLevel)
        {
            osg::Matrix mtx = computeTexMat(itor->first, id);
            pTexMat = new osg::TexMat;
            pTexMat->setMatrix(mtx);
        }
        vecTexture.push_back(std::make_pair(pTexture2D, pTexMat));

        //若是底层影像瓦片，需要加入到缓存池中
        if(itor->second)
        {
            m_pSharedObjectPool->addObject(itor->first, pTexture2D);
        }

        //所没有透明通道，则不需要读取其他的瓦片
        if(pTexture2D->getUserData() == NULL)
        {
            return;
        }
    }
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readDEMTileLayerByID(const ID &id, const osgDB::Options *pOptions) const
{
    if(id.TileID.m_nType != TERRAIN_TILE_HEIGHT_FIELD)
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }
    std::vector<LayerItem> vecLayersOrder;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxDemCoverOrder);
        if(m_vecDemCoverOrder.empty())
        {
            return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
        }
        vecLayersOrder.resize(m_vecDemCoverOrder.size());
        std::transform(m_vecDemCoverOrder.begin(), m_vecDemCoverOrder.end(), vecLayersOrder.begin(), TerrainItemFlagFetcher());
    }

    bool bHasAlpha = true;
    bool bIsShared = true;

    //判断当前ID在每一层压盖中是否为底层瓦片，并且找到其对应的最近的瓦片
    //unsigned int nCount = vecLayersOrder.size();
    std::vector<std::pair<ID, bool> > vecNearestID;
    for(unsigned int i = 0u; i < vecLayersOrder.size(); i++)
    {
        ID nearest_id;
        bool bIsBottomTile = false;
        ID temp_id = id;
        temp_id.TileID.m_nDataSetCode = vecLayersOrder[i].m_nDatasetCode;
        temp_id.TileID.m_nUniqueID    = vecLayersOrder[i].m_nUniqueID;
        findNearestIDbyID(temp_id, bIsBottomTile, nearest_id);

        if(!nearest_id.isValid())
        {
            continue;
        }

        //最近ID是有效的并且不是最底层的瓦片时，说明此瓦片不能被共享
        if(nearest_id.isValid() && !bIsBottomTile)
        {
            bIsShared = false;
        }
        vecNearestID.push_back(std::make_pair(nearest_id, bIsBottomTile));
    }

    const unsigned int nCount = vecNearestID.size();

    ID temp_id(0ui64, 0ui64, 0ui64);
    osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
    osg::ref_ptr<osg::Image> pResultImage, pTempImage;

    //当前ID处于每层压盖的底层瓦片之下，可以使用共享机制
    if(bIsShared)
    {
        //找到最近的底层瓦片ID，从此ID对应的瓦片开始是共享的
        for(unsigned int i = 0; i < nCount; i++)
        {
            if(!vecNearestID[i].first.isValid())
            {
                continue;
            }

            if(!temp_id.isValid())
            {
                temp_id = vecNearestID[i].first;
                continue;
            }
            if(temp_id.TileID.m_nLevel < vecNearestID[i].first.TileID.m_nLevel)
            {
                temp_id = vecNearestID[i].first;
            }
        }

        //共享瓦片的ID应该为2
        temp_id.TileID.m_nDataSetCode = 2u;

        //读取此瓦片，若读到，直接返回
        pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(temp_id, pOptions, NULL).getImage();
        if(pTempImage.valid())
        {
            return osgDB::ReaderWriter::ReadResult(pTempImage.get());
        }
        //未读取到，创建并加入缓存池
        else
        {
            for(unsigned int i = 0u; i < nCount; i++)
            {
                if(!vecNearestID[i].first.isValid())
                {
                    continue;
                }

                //读取一层的底层瓦片
                pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(vecNearestID[i].first, pOptions, NULL).getImage();

                if(!pTempImage.valid())
                {
                    continue;
                }

                //裁剪读取到的瓦片到当前瓦片的范围
                pTempImage = floodImage(vecNearestID[i].first, id, pTempImage);

                if(pResultImage.valid())
                {
                    pResultImage = combineImage(pResultImage.get(), pTempImage.get(), false);
                }
                else
                {
                    pResultImage = pTempImage;
                }

                if(!doesImageHaveAlpha(pResultImage.get()))
                {
                    bHasAlpha = false;
                    break;
                }
            }
        }
    }
    //不能立即从共享池中去找
    else
    {
        for(unsigned int i = 0u; i < nCount; i++)
        {
            if(!vecNearestID[i].first.isValid())
            {
                continue;
            }

            //最底层的ID，尝试缓存
            if(vecNearestID[i].second)
            {
                //缓存池中查找
                temp_id = vecNearestID[i].first;
                temp_id.TileID.m_nDataSetCode = 2;
                pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(temp_id, pOptions, NULL).getImage();
                if(!pTempImage.valid())
                {
                    pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(vecNearestID[i].first, pOptions, NULL).getImage();
                    if(pTempImage.valid())
                    {
                        pTempImage->setID(temp_id);
                        pPool->addObject(temp_id, pTempImage.get());
                    }
                }
            }
            else
            {
                pTempImage = const_cast<FileReadInterceptor *>(this)->readImage(vecNearestID[i].first, pOptions, NULL).getImage();
            }

            if(!pTempImage.valid())
            {
                continue;
            }

            pTempImage = floodImage(vecNearestID[i].first, id, pTempImage);

            if(pResultImage.valid())
            {
                pResultImage = combineImage(pResultImage.get(), pTempImage.get(), false);
            }
            else
            {
                pResultImage = pTempImage;
            }

            if(!doesImageHaveAlpha(pResultImage.get()))
            {
                bHasAlpha = false;
                break;
            }
        }
    }

    if(!pResultImage.valid())
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }
    if((pResultImage->getDataType() != GL_FLOAT) || (pResultImage->getPixelFormat() != GL_LUMINANCE))
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }

    if(bHasAlpha)
    {
        clearAlphaAsColor(pResultImage.get(), 0.0f);
    }

    smoothHeightField(pResultImage.get(), 3u);

    const osg::Vec2s vecSize1(pResultImage->s(), pResultImage->t());
    osg::Vec2s vecSize2;
    findNearestImageSize(vecSize1, vecSize2);
    if(vecSize2 != vecSize1)
    {
        OpenSP::sp<cmm::image::Image>   pImage = new cmm::image::Image;
        pImage->attach(pResultImage->data(), pResultImage->s(), pResultImage->t(), cmm::image::PF_LUMINANCE);

        const int nOffsetX = (vecSize1.x() - vecSize2.x()) / 2;
        const int nOffsetY = (vecSize1.y() - vecSize2.y()) / 2;
        OpenSP::sp<cmm::image::Image>   pSubImage = dynamic_cast<cmm::image::Image*>(pImage->getSubImage(nOffsetX, nOffsetY, vecSize2.x(), vecSize2.y()));
        pImage->deatch();

        osg::ref_ptr<osg::Image> pOsgImage = new osg::Image;
        pOsgImage->allocateImage(vecSize2.x(), vecSize2.y(), 1, GL_LUMINANCE, GL_FLOAT);
        memcpy(pOsgImage->data(), pSubImage->data(), pOsgImage->getImageSizeInBytes());

        pSubImage = NULL;
        pResultImage = pOsgImage;
    }

    return osgDB::ReaderWriter::ReadResult(pResultImage.get());
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readDetailByID(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo) const
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);

    if(pCreationInfo == NULL)
    {
        return rr;
    }

    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = readFromLocalDB(id, pBuffer, nLength);
    if(!bFromLocal && m_pDEUNetwork.valid())
    {
        if(!m_pDEUNetwork->queryData(id, pBuffer, nLength))
        {
            return rr;
        }
    }

    MemoryEraser eraser(bFromLocal, pBuffer);

    bson::bsonDocument  bsonDoc;
    bson::bsonStream    bs;
    bs.Write(pBuffer, nLength);
    bs.Reset();
    bsonDoc.Read(&bs);

    OpenSP::sp<param::IDetail> pIDetail = param::createDetail(id);
    if(!pIDetail.valid())
    {
        return rr;
    }

    if(!pIDetail->fromBson(bsonDoc))
    {
        return rr;
    }

    param::Detail *pDetail = dynamic_cast<param::Detail *>(pIDetail.get());
    if(!pDetail)
    {
        return rr;
    }

    const param::Detail::CreationInfo *pInfo = dynamic_cast<const param::Detail::CreationInfo *>(pCreationInfo);
    if(!pInfo)
    {
        return rr;
    }

    osg::ref_ptr<osg::Node> pNode = pDetail->createDetailNode(pInfo);
    if(!pNode.valid()) return rr;

    pNode->setID(id);
    rr = pNode.get();

    return rr;
}

osgDB::ReaderWriter::ReadResult FileReadInterceptor::readParamByID(const ID &id, const osgDB::Options *pOptions) const
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);

    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = readFromLocalDB(id, pBuffer, nLength);
    if(!bFromLocal && m_pDEUNetwork.valid())
    {
        if(!m_pDEUNetwork->queryData(id, pBuffer, nLength))
        {
            if (Registry::instance()->getUseShadow())
            {
                rr.getNode()->setNodeMask(1);
            }
            return rr;
        }
    }

    MemoryEraser eraser(bFromLocal, pBuffer);

    bson::bsonDocument  bsonDoc;
    bson::bsonStream    bs;
    bs.Write(pBuffer, nLength);
    bs.Reset();
    bsonDoc.Read(&bs);

    OpenSP::sp<param::IParameter> pIParameter = param::createParameter(id);
    if(!pIParameter.valid())
    {
        return rr;
    }

    OpenSP::sp<param::Parameter>    pParameter = dynamic_cast<param::Parameter *>(pIParameter.get());
    if(!pParameter->fromBson(bsonDoc))
    {
        return rr;
    }


    osg::ref_ptr<osg::Node> pNode = pParameter->createParameterNode();
    if(!pNode.valid()) return rr;

    pNode->setID(id);
    rr = pNode.get();

    if(!m_pStateManager.valid())
    {
        return rr;
    }

    std::map<std::string, bool> objectstates;
    OpenSP::sp<cmm::IStateQuerier> pStateQuerier;
    if(!m_pStateQuerier.lock(pStateQuerier))
    {
        return rr;
    }

    m_pStateQuerier->getObjectStates(id, objectstates);
    for(std::map<std::string, bool>::const_iterator itor = objectstates.begin(); itor != objectstates.end(); ++itor)
    {
        const std::string &strStateType = itor->first;
        if(strStateType == cmm::STATE_VISIBLE)
        {
            continue;
        }

        StateBase *pState = dynamic_cast<StateBase *>(m_pStateManager->getRenderState(strStateType));
        if(!pState)         continue;
        if(!itor->second)   continue;

        pState->applyState(rr.getNode(), true);
    }

    if (Registry::instance()->getUseShadow())
    {
        rr.getNode()->setNodeMask(1);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readTileFragmentByActualID(const ID &id, const osgDB::Options *pOptions) const
{
    ID nearest_id;
    bool bIsBottomTile = false;
    if(!findNearestIDbyID(id, bIsBottomTile, nearest_id))
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }

    osg::ref_ptr<osg::Image> pImage = const_cast<FileReadInterceptor *>(this)->readImage(nearest_id, pOptions, NULL).getImage();

    if(!pImage.valid())
    {
        return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    }

    if(bIsBottomTile)
    {
        pImage->setID(nearest_id);
        osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
        pPool->addObject(nearest_id, pImage.get());
    }

    //若有Alpha通道，需要裁剪
    if(doesImageHaveAlpha(pImage))
    {
        pImage = floodImage(nearest_id, id, pImage.get());
    }
    //pImage = floodImage(m_Pyramid, idFetch, id, pImage.get());
    //if(!pImage.valid())
    //{
    //    return osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    //}
    return osgDB::ReaderWriter::ReadResult(pImage.get());
}


bool FileReadInterceptor::readFromLocalDB(const ID &id, void *&pBuffer, unsigned &nLength) const
{
    std::vector<OpenSP::sp<deudbProxy::IDEUDBProxy> >     vecTargetLocalDBs;
    if(m_pLocalTempDB.valid())
    {
        vecTargetLocalDBs.push_back(m_pLocalTempDB);
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxLocalDBs);
        LocalDBMap::const_iterator itorFind = m_mapLocalDBs.find(id.ObjectID.m_nDataSetCode);
        if(itorFind != m_mapLocalDBs.end())
        {
            const std::vector<LocalDB> &vecLocalDBs = itorFind->second;
            vecTargetLocalDBs.reserve(vecLocalDBs.size());
            std::vector<LocalDB>::const_iterator itorLocalDB = vecLocalDBs.begin();
            for( ; itorLocalDB != vecLocalDBs.end(); ++itorLocalDB)
            {
                const LocalDB &dbLocal = *itorLocalDB;

                vecTargetLocalDBs.push_back(dbLocal.m_pDeuDB);
            }
        }
    }

    std::vector<OpenSP::sp<deudbProxy::IDEUDBProxy> >::iterator itorDB = vecTargetLocalDBs.begin();
    for( ; itorDB != vecTargetLocalDBs.end(); ++itorDB)
    {
        deudbProxy::IDEUDBProxy *pLocalDB = itorDB->get();
        if(pLocalDB->readBlock(id, pBuffer, nLength))
        {
            return true;
        }
    }

    return false;
}


bool FileReadInterceptor::findNearestIDbyID(const ID &id, bool &bIsBottomTile, ID &nearest_id) const
{
    OpenThreads::Mutex *pMutex = NULL;
    bool bIsDEM = false;
    if(id.TileID.m_nType == TERRAIN_TILE_HEIGHT_FIELD)
    {
        pMutex = &m_mtxDemTopTiles;
        bIsDEM = true;
    }
    else if(id.TileID.m_nType == TERRAIN_TILE_IMAGE)
    {
        pMutex = &m_mtxDomTopTiles;
        bIsDEM = false;
    }
    else return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(*pMutex);

    const std::map<unsigned __int64, TerrainTilesInfo> &mapTopTiles = bIsDEM ? m_mapDemTopTiles : m_mapDomTopTiles;

    const unsigned __int64 nUniqueID = id.TileID.m_nUniqueID;
    std::map<unsigned __int64, TerrainTilesInfo>::const_iterator itorFind = mapTopTiles.find(nUniqueID);
    if(itorFind == mapTopTiles.end())
    {
        // currently it has no relevant dataset
        return false;
    }

    const TerrainTilesInfo &mapTilesInfo = itorFind->second;
    if(mapTilesInfo.empty())
    {
        // currently it has no top tile in this dataset
        return false;
    }

    const std::pair<ID, unsigned> &firstTileInTerrain = *mapTilesInfo.begin();
    const unsigned nMinLevelOfTerrain = firstTileInTerrain.first.TileID.m_nLevel;
    if(id.TileID.m_nLevel < nMinLevelOfTerrain)
    {
        // the level of the request tile is too small, even the top level of the relevant dataset cannot fit it
        return false;
    }

    ID idExpectedTop = id;
    if(id.TileID.m_nLevel > nMinLevelOfTerrain)
    {
        unsigned nRowParent = 0u, nColParent = 0u;
        const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
        if(!pPyramid->getParentByLevel(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, nMinLevelOfTerrain, nRowParent, nColParent))
        {
            return false;
        }
        idExpectedTop.TileID.m_nRow   = (unsigned  int)nRowParent;
        idExpectedTop.TileID.m_nCol   = (unsigned  int)nColParent;
        idExpectedTop.TileID.m_nLevel = (unsigned char)firstTileInTerrain.first.TileID.m_nLevel;
    }

    TerrainTilesInfo::const_iterator itorFindTile = mapTilesInfo.find(idExpectedTop);
    if(itorFindTile == mapTilesInfo.end())
    {
        // the request tile cannot be fit in the relevant dataset, because it is not covered by any top tile of the relevant dataset
        return false;
    }

    bIsBottomTile = false;
    nearest_id = id;
    const unsigned nMaxLevel = itorFindTile->second;
    if(id.TileID.m_nLevel > nMaxLevel)
    {
        unsigned nTargetRow = 0u, nTargetCol = 0u;
        const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
        if(!pPyramid->getParentByLevel(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, nMaxLevel, nTargetRow, nTargetCol))
        {
            return false;
        }
        nearest_id.TileID.m_nLevel = nMaxLevel;
        nearest_id.TileID.m_nRow   = nTargetRow;
        nearest_id.TileID.m_nCol   = nTargetCol;

        //bIsBottomTile = true;
    }

    if(nearest_id.TileID.m_nLevel == nMaxLevel)
    {
        bIsBottomTile = true;
    }

    return true;
}


osg::Image *FileReadInterceptor::parseImageFromStream(const void *pBuffer, unsigned nLength, const osgDB::Options *pOptions) const
{
    if(!pBuffer || nLength < 1u)    return NULL;

    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    std::strstream ss((char *)pBuffer, nLength);

    osg::ref_ptr<osgDB::ReaderWriter>   pImageReaderWriter;
    const cmm::image::ImageType eType = cmm::image::getImageStreamType(pBuffer);
    switch(eType)
    {
    case cmm::image::TYPE_JPG:
        pImageReaderWriter = m_pJpgReaderWriter;
        break;
    case cmm::image::TYPE_PNG:
        pImageReaderWriter = m_pPngReaderWriter;
        break;
    case cmm::image::TYPE_BMP:
        pImageReaderWriter = m_pBmpReaderWriter;
        break;
    case cmm::image::TYPE_TIF:
        pImageReaderWriter = m_pTifReaderWriter;
        break;
    case cmm::image::TYPE_DDS:
        pImageReaderWriter = m_pDdsReaderWriter;
        break;
    default:    break;
    }

    if(!pImageReaderWriter.valid()) return NULL;

    rr = pImageReaderWriter->readImage(ss, pOptions);
    if(!rr.validImage())    return NULL;

    osg::ref_ptr<osg::Image>    pImage = rr.takeImage();
    return pImage.release();
}


bool FileReadInterceptor::addWMTSTileSet(deues::ITileSet *pTileSet)
{
    if(!pTileSet)   return false;

    const ID &id = pTileSet->getID();
    m_mapWMTSTileSet[id] = pTileSet;

    m_pDEUNetwork->addTileSet(pTileSet);
    return true;
}


bool FileReadInterceptor::removeWMTSTileSet(deues::ITileSet *pTileSet)
{
    if(!pTileSet)   return false;

    const ID &id = pTileSet->getID();
    m_mapWMTSTileSet.erase(id);

    m_pDEUNetwork->removeTileSet(pTileSet);
    return true;
}

