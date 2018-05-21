#include "TileBuilderImpl.h"
#include "TaskQueue.h"
#include "TilingThreadPool.h"
#include "Common/Pyramid.h"
#include "gdal_priv.h"
#include <Common/DEUBson.h>
#include <EventAdapter/IEventObject.h>
#include <IDProvider/Definer.h>
#include <Common\deuMath.h>
#include <osgDB\AuthenticationMap>
#include <osgDB\ObjectWrapper>
#include <algorithm>
#include <map>
#include <fstream>

using namespace std;


#include "DEUCheck.h"
DEUCheck checker(18, 6);

#define MAX_THREAD_COUNT 10

ITileBuilder* createTileBuilder()
{
    OpenSP::sp<TileBuilderImpl> pFileCache = new TileBuilderImpl;
    return pFileCache.release();
}

bool compareMethod(OpenSP::sp<Source> &source1, OpenSP::sp<Source> &source2)
{
    const cmm::math::Point2d& vecPrecision1 = source1->getPrecision();
    const cmm::math::Point2d& vecPrecision2 = source2->getPrecision();
    double precision1 = vecPrecision1.x()*vecPrecision1.x() + vecPrecision1.y()*vecPrecision1.y();
    double precision2 = vecPrecision2.x()*vecPrecision2.x() + vecPrecision2.y()*vecPrecision2.y();

    return precision1 < precision2;
}

TileBuilderImpl::TileBuilderImpl(void)
: m_nTileSize(256)
, m_nDataSetCode(1)
, m_nMinLevel(1)
, m_nMaxLevel(31)
, m_nMinInterval(1)
, m_nMaxInterval(31)
, m_bHeightField(false)
, m_bAutoComputeLevel(true)
, m_pEventAdapter(NULL)
, m_pTilingThreadPool(NULL)
, m_bStop(false)
{
    GDALAllRegister();

    m_strTargetDB = "";
    m_vecSourceFile.clear();

    m_pTilingThreadPool = new TilingThreadPool;
    m_pTaskQueue = new TaskQueue();
}

TileBuilderImpl::~TileBuilderImpl(void)
{
    if (m_vecSourceFile.size() > 0)
    {
        m_vecSourceFile.clear();
    }

    m_pTilingThreadPool = NULL;

    m_pTaskQueue = NULL;
}

void TileBuilderImpl::setAutoComputeLevel(const bool bCompute)
{
    m_bAutoComputeLevel = bCompute;

    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();
}

bool TileBuilderImpl::getAutoComputeLevel()
{
    return m_bAutoComputeLevel;
}

void TileBuilderImpl::setDEM(const bool isDem)
{
    m_bHeightField = isDem;

    m_pTaskQueue->setHeightField(isDem);

    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();
}

bool TileBuilderImpl::getDEM()
{
    return m_bHeightField;
}

bool TileBuilderImpl::setMinLevel(const unsigned nLevel)
{
    return setLevel(nLevel, m_nMaxLevel);
}

unsigned TileBuilderImpl::getMinLevel()
{
    return m_nMinLevel;
}

bool TileBuilderImpl::setMaxLevel(const unsigned nLevel)
{
    return setLevel(m_nMinLevel, nLevel);
}

unsigned TileBuilderImpl::getMaxLevel()
{
    return m_nMaxLevel;
}

unsigned TileBuilderImpl::getMinInterval()
{
    return m_nMinInterval;
}

unsigned TileBuilderImpl::getMaxInterval()
{
    return m_nMaxInterval;
}

bool TileBuilderImpl::setTileSize(const unsigned nTileSize)
{
    int nAugmenter = 4;

    while (nAugmenter>=4 && nAugmenter<=256)
    {
        if (nTileSize == 2*nAugmenter)
        {
            m_nTileSize = nTileSize;
            m_pTaskQueue->setTileSize(nTileSize);

            computeMinInterval();
            computeMaxInterval();
            computeMinMaxLevel();

            return true;
        }
        else
        {
            nAugmenter *= 2;
        }
    }

    return false;
}

unsigned TileBuilderImpl::getTileSize()
{
    return m_nTileSize;
}

void TileBuilderImpl::setDataSetCode(const unsigned nDataSetCode)
{
    m_nDataSetCode = nDataSetCode;

    m_pTaskQueue->setDataSetCode(nDataSetCode);
}

unsigned TileBuilderImpl::getDataSetCode()
{
    return m_nDataSetCode;
}

void TileBuilderImpl::setTargetDB(const std::string& strDB)
{
    m_strTargetDB = strDB;
}

void TileBuilderImpl::getTargetDB(std::string& strDB)
{
    strDB = m_strTargetDB;
}

bool TileBuilderImpl::addSourceFile(const std::string &strFileName)
{
    OpenSP::sp<Source> pSource = new Source();
    if (pSource == NULL)
    {
        return false;
    }

    if (pSource->create(strFileName, m_bHeightField, this))
    {
        m_vecSourceFile.push_back(pSource);
    }
    else
    {
        return false;
    }

    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();

    return true;
}

bool TileBuilderImpl::getFileCoordinates(const std::string &strFileName, double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
    for (size_t iSize=0; iSize<m_vecSourceFile.size(); iSize++)
    {
        if (m_vecSourceFile[iSize]->getSourceFileName() == strFileName)
        {
            m_vecSourceFile[iSize]->getFileCoordinates(dMinX, dMinY, dMaxX, dMaxY);
        }
    }

    return true;
}

bool TileBuilderImpl::setFileCoordinates(const std::string &strFileName, double dMinX, double dMinY, double dMaxX, double dMaxY)
{
    for (size_t iSize=0; iSize<m_vecSourceFile.size(); iSize++)
    {
        if (m_vecSourceFile[iSize]->getSourceFileName() == strFileName)
        {
            m_vecSourceFile[iSize]->setFileCoordinates(dMinX, dMinY, dMaxX, dMaxY);
        }
    }

    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();

    return true;
}

bool TileBuilderImpl::getSourceFile(const unsigned nIndex, std::string& strFileName)
{
    if (nIndex >= m_vecSourceFile.size())
    {
        return false;
    }

    strFileName = m_vecSourceFile[nIndex]->getSourceFileName();

    return true;
}

bool TileBuilderImpl::removeSourceFile(const unsigned nIndex)
{
    if (nIndex >= m_vecSourceFile.size())
    {
        return false;
    }

    unsigned i = 0;
    std::vector<OpenSP::sp<Source>>::iterator it = m_vecSourceFile.begin();
    for (; it != m_vecSourceFile.end(); ++it)
    {
        if (nIndex == i)
        {
            m_vecSourceFile.erase(it);
            break;
        }

        i++;
    }

    if (m_vecSourceFile.size() == 0)
    {
        m_nMinInterval = 1;
        m_nMaxInterval = 31;

        return true;
    }

    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();

    return true;
}

unsigned TileBuilderImpl::getSourceFileCount()
{
    return m_vecSourceFile.size();
}

void TileBuilderImpl::clearSourceFile()
{
    m_vecSourceFile.clear();

    m_nMinInterval = 1;
    m_nMaxInterval = 31;
}

bool TileBuilderImpl::setLevel(const unsigned int nMinLevel, const unsigned int nMaxLevel)
{
    if (nMinLevel >= m_nMinInterval && nMinLevel <= m_nMaxInterval && nMaxLevel >= m_nMinInterval && nMaxLevel <= m_nMaxInterval && nMinLevel <= nMaxLevel)
    {
        m_nMinLevel = nMinLevel;
        m_nMaxLevel = nMaxLevel;

        return true;
    }

    if (nMinLevel == 0xFFFFFFFF && nMaxLevel == 0xFFFFFFFF)
    {
        computePyramidInfo();

        return true;
    }
    else if (nMinLevel == 0xFFFFFFFF)
    {
        unsigned int nMin = computeMinInterval();
        if (nMin >= 1 && nMin <= 31 && nMin <= nMaxLevel)
        {
            m_nMinLevel = nMin;
            m_nMaxLevel = nMaxLevel;

            return true;
        }
    }
    else if (nMaxLevel == 0xFFFFFFFF)
    {
        unsigned int nMax = computeMaxInterval();
        if (nMax >= 1 && nMax <= 31 && nMinLevel <= nMax)
        {
            m_nMinLevel = nMinLevel;
            m_nMaxLevel = nMax;

            return true;
        }
    }

    return false;
}

void TileBuilderImpl::getLevel(unsigned int &nMinLevel, unsigned int &nMaxLevel)
{
    nMinLevel = m_nMinLevel;
    nMaxLevel = m_nMaxLevel;

    return;
}

void TileBuilderImpl::setEventAdapter(ea::IEventAdapter* pEventAdapter)
{
    m_pEventAdapter = pEventAdapter;

    return;
}

unsigned int TileBuilderImpl::computeMinInterval()
{
    if (m_vecSourceFile.size() < 1)
    {
        return -1;
    }

    for (size_t iSize=0; iSize<m_vecSourceFile.size(); iSize++)
    {
        m_vecSourceFile[iSize]->computeMinInterval();
    }

    //找到最小层号，必须保证每个文件至少有MINPIXEL个像素
    m_nMinInterval = m_vecSourceFile[0]->getMinInterval();

    for (size_t iSize=1; iSize<m_vecSourceFile.size(); iSize++)
    {
        unsigned int nMinFileInterval = m_vecSourceFile[iSize]->getMinInterval();
        m_nMinInterval = m_nMinInterval > nMinFileInterval ? m_nMinInterval : nMinFileInterval;
    }

    return 0;
}

unsigned int TileBuilderImpl::computeMaxInterval()
{
    if (m_vecSourceFile.size() < 1)
    {
        return -1;
    }

    if (!m_bAutoComputeLevel)
    {
        m_nMaxInterval = 31;
        return 0;
    }

    for (size_t iSize=0; iSize<m_vecSourceFile.size(); iSize++)
    {
        m_vecSourceFile[iSize]->computeMaxInterval();
    }

    //找到最小层号，必须保证每个文件至少有MINPIXEL个像素
    m_nMaxInterval = m_vecSourceFile[0]->getMaxInterval();

    for (size_t iSize=1; iSize<m_vecSourceFile.size(); iSize++)
    {
        unsigned int nMaxFileInterval = m_vecSourceFile[iSize]->getMaxInterval();
        m_nMaxInterval = m_nMaxInterval > nMaxFileInterval ? m_nMaxInterval : nMaxFileInterval;
    }

    return 0;
}

unsigned int TileBuilderImpl::computeMinMaxLevel()
{
    if (m_nMinLevel < m_nMinInterval)
    {
        m_nMinLevel = m_nMinInterval;
    }

    if (m_nMaxLevel > m_nMaxInterval)
    {
        m_nMaxLevel = m_nMaxInterval;
    }

    if (m_nMinLevel > m_nMaxLevel)
    {
        m_nMaxLevel = m_nMinLevel;
    }

    return 0;
}

bool TileBuilderImpl::computePyramidInfo()
{
    computeMinInterval();
    computeMaxInterval();
    computeMinMaxLevel();

    return true;
}

bool TileBuilderImpl::process()
{
    if (m_vecSourceFile.size() < 1)
    {
        return false;
    }

    if (m_pTaskQueue == NULL || m_pTilingThreadPool == NULL || m_pEventAdapter == NULL)
    {
        return false;
    }

    m_bStop = false;

    getInvalidColor();

    m_pTargetDB = deudbProxy::createDEUDBProxy();
    if (!m_pTargetDB->openDB(m_strTargetDB, 16*1024*1024))
    {
        return false;
    }

    m_pTaskQueue->setGlobeUniqueNumber();
    m_pTaskQueue->setTopLevel(m_nMinLevel);
    m_pTaskQueue->setIEventAdapter(m_pEventAdapter.get());
    m_pTaskQueue->setInvalidColor(m_vecInvalidColor);

    //创建线程池
    if (!m_pTilingThreadPool->initialize(MAX_THREAD_COUNT, m_pTaskQueue.get(), m_pTargetDB.get()))
    {
        return false;
    }
    m_pTilingThreadPool->startMission();

    //整个地形的区域矩形;
    const cmm::math::Box2d &area = m_vecSourceFile[0]->getBound();
    m_TotalAreaBound.set(area.point0(), area.point1());
    for (size_t iSize=1; iSize<m_vecSourceFile.size(); iSize++)
    {
        const cmm::math::Box2d &area = m_vecSourceFile[iSize]->getBound();
        cmm::math::Point2d ptMin, ptMax;
        ptMin.x() = std::min(m_TotalAreaBound.point0().x(), area.point0().x());
        ptMin.y() = std::min(m_TotalAreaBound.point0().y(), area.point0().y());
        ptMax.x() = std::max(m_TotalAreaBound.point1().x(), area.point1().x());
        ptMax.y() = std::max(m_TotalAreaBound.point1().y(), area.point1().y());
        m_TotalAreaBound.set(ptMin, ptMax);
    }

    //按照精度，将源文件排序，精度从高到低，解决完全覆盖的问题
    std::sort(m_vecSourceFile.begin(), m_vecSourceFile.end(), compareMethod);

    //为进度条设置信息
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    if (pPyramid == NULL)
    {
        return false;
    }

    unsigned int nRange = 0;
    for (unsigned int nLevel=m_nMaxLevel; nLevel>=m_nMinLevel; nLevel--)
    {
        unsigned nMinRow, nMinCol;
        pPyramid->getTile(nLevel, m_TotalAreaBound.point0().x(), m_TotalAreaBound.point0().y(), nMinRow, nMinCol);

        unsigned nMaxRow, nMaxCol;
        pPyramid->getTile(nLevel, m_TotalAreaBound.point1().x(), m_TotalAreaBound.point1().y(), nMaxRow, nMaxCol);

        nRange += (nMaxRow - nMinRow + 1) * (nMaxCol - nMinCol + 1);
    }

    unsigned int nState  = 1;
    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
    if (pEventObject == NULL)
    {
        return false;
    }
    pEventObject->setAction(ea::ACTION_TILE_BUILDER);
    pEventObject->putExtra("STATE", nState);
    pEventObject->putExtra("Range", nRange);
    m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());

    //开始切片任务
    for (unsigned int i=m_nMaxLevel; i>=m_nMinLevel; i--)
    {
        tileBuildLevel(i);
    }

    m_pTilingThreadPool->finishMission(false);

    //写日志文件
    if (!m_bStop)
    {
        writeConfigurationFile();
    }

    if (m_pTargetDB != NULL)
    {
        m_pTargetDB->closeDB();
        m_pTargetDB = NULL;
    }

    return true;
}


void TileBuilderImpl::stop()
{
    if (m_pTaskQueue == NULL || m_pTilingThreadPool == NULL)
    {
        return;
    }

    m_bStop = true;

    m_pTaskQueue->Stop();

    m_pTilingThreadPool->finishMission(true);

    return;
}

#include <Windows.h>
bool TileBuilderImpl::addInvalidColor(const std::string &strInvalidColor)
{
    //读写文件InvalidColor.ini
    char szAppPath[1024] = {0};
    ::GetModuleFileNameA(NULL, szAppPath, 1024);

    std::string strFileName = szAppPath;
    strFileName = strFileName.substr(0, strFileName.find_last_of('\\')) + "\\InvalidColor.ini";
    std::ifstream file;

    file.open(strFileName, ios::in);
    if (!file.is_open())
    {
        return false;
    }

    unsigned int nBegin = file.tellg();
    file.seekg (0, ios::end);
    unsigned int nEnd = file.tellg();
    unsigned int nLen = nEnd - nBegin;
    file.seekg (0, ios::beg);
    char* pbuf = new char[nLen + 1];
    memset(pbuf, 0, nLen + 1);
    file.read(pbuf, nLen);
    file.close();

    bson::bsonDocument bsonDoc;
    bsonDoc.FromJsonString(pbuf);
    delete [] pbuf;
    pbuf = NULL;

    if(!bsonDoc.AddStringElement("InvalidCorlor",strInvalidColor.data()))
    {
        return false;
    }

    unsigned int m = bsonDoc.ChildCount();
    char strNum[5];
    itoa(m - 1,strNum,10);
    bson::bsonStringEle* pUserInvalidColor = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("UseInvalidColor"));
    pUserInvalidColor->SetStrValue(strNum);

    std::string strTemp;
    bsonDoc.JsonString(strTemp);
    std::ofstream filewrite;
    filewrite.open(strFileName, ios::in | ios::out);
    if (!filewrite.is_open())
    {
        return false;
    }
    filewrite<<strTemp;
    filewrite.close();
    return true;
}

bool TileBuilderImpl::delInvalidColor(const std::string &strInvalidColor)
{
    char szAppPath[1024] = {0};
    ::GetModuleFileNameA(NULL, szAppPath, 1024);

    std::string strFileName = szAppPath;
    strFileName = strFileName.substr(0, strFileName.find_last_of('\\')) + "\\InvalidColor.ini";
    std::ifstream file;

    file.open(strFileName, ios::in);
    if (!file.is_open())
    {
        return false;
    }

    unsigned int nBegin = file.tellg();
    file.seekg (0, ios::end);
    unsigned int nEnd = file.tellg();
    unsigned int nLen = nEnd - nBegin;
    file.seekg (0, ios::beg);
    char* pbuf = new char[nLen + 1];
    memset(pbuf, 0, nLen + 1);
    file.read(pbuf, nLen);
    file.close();

    bson::bsonDocument bsonDoc;
    bsonDoc.FromJsonString(pbuf);
    delete [] pbuf;
    pbuf = NULL;

    unsigned nFirst = strInvalidColor.find_first_of(',');
    unsigned nLast  = strInvalidColor.find_last_of(',');

    unsigned int strR = atoi((strInvalidColor.substr(0, nFirst)).c_str());
    unsigned int strG = atoi((strInvalidColor.substr(nFirst + 1, nLast - nFirst - 1)).c_str());
    unsigned int strB = atoi((strInvalidColor.substr(nLast + 1, strInvalidColor.length() - nLast  - 1)).c_str());

    for (int i = 0; i < bsonDoc.ChildCount(); i++)
    {
        std::string strName;
        std::string strValue;

        strName = bsonDoc.GetElement(i)->EName();
        if(strName != "InvalidCorlor") continue;

        bsonDoc.GetElement(i)->ValueString(strValue);
        unsigned nFirst = strValue.find_first_of(',');
        unsigned nLast  = strValue.find_last_of(',');
        if (nFirst == strValue.length() || nLast == strValue.length() || nLast <= nFirst) continue;

        unsigned int nR = atoi((strValue.substr(1, nFirst - 1)).c_str());
        unsigned int nG = atoi((strValue.substr(nFirst + 1, nLast - nFirst - 1)).c_str());
        unsigned int nB = atoi((strValue.substr(nLast + 1, strValue.length() - nLast - 1 - 1)).c_str());

        if (nR == strR && nG == strG && nB == strB)
        {
            bsonDoc.DelElement(i);
        }
    }

    unsigned int m = bsonDoc.ChildCount();
    char strNum[5];
    itoa(m - 1,strNum,10);
    bson::bsonStringEle* pUserInvalidColor = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("UseInvalidColor"));
    pUserInvalidColor->SetStrValue(strNum);

    std::string strTemp;
    bsonDoc.JsonString(strTemp);
    std::ofstream filewrite;
    filewrite.open(strFileName, ios::out);
    if (!filewrite.is_open())
    {
        return false;
    }

    filewrite<<strTemp;
    filewrite.close();
    return true;
}

std::vector<INVALIDCOLOR>&  TileBuilderImpl::getAllInvalidColor()
{
    getInvalidColor();
    return m_vecInvalidColor;
}

std::vector<INVALIDCOLOR>&  TileBuilderImpl::getVecInvalidColor()
{
    return m_vecInvalidColor;
}

bool TileBuilderImpl::tileBuildLevel(unsigned int nLevel)
{
    //根据TotalAreaBound计算出在本level中的最小和最大的行列号
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    if (pPyramid == NULL)
    {
        return false;
    }

    unsigned nMinRow, nMinCol;
    pPyramid->getTile(nLevel, m_TotalAreaBound.point0().x(), m_TotalAreaBound.point0().y(), nMinRow, nMinCol);

    unsigned nMaxRow, nMaxCol;
    pPyramid->getTile(nLevel, m_TotalAreaBound.point1().x(), m_TotalAreaBound.point1().y(), nMaxRow, nMaxCol);

    for(unsigned row = nMinRow; row <= nMaxRow; row++)
    {
        tileBuildRow(nLevel, row);
    }

    return true;
}


bool TileBuilderImpl::tileBuildRow(unsigned int nLevel, unsigned int nRow)
{
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    if (pPyramid == NULL)
    {
        return false;
    }

    unsigned nMinRow, nMinCol;
    pPyramid->getTile(nLevel, m_TotalAreaBound.point0().x(), m_TotalAreaBound.point0().y(), nMinRow, nMinCol);

    unsigned nMaxRow, nMaxCol;
    pPyramid->getTile(nLevel, m_TotalAreaBound.point1().x(), m_TotalAreaBound.point1().y(), nMaxRow, nMaxCol);

    // 遍历本行的所有瓦片，逐个生产
    for(unsigned col = nMinCol; col <= nMaxCol; col++)
    {
        tileBuild(nLevel, nRow, col);
    }

    return true;
}

bool TileBuilderImpl::tileBuild(unsigned int nLevel, unsigned int nRow, unsigned int nCol)
{
    if (m_bStop || m_pTaskQueue == NULL || m_pTilingThreadPool == NULL)
    {
        return false;
    }

    OpenSP::sp<TilingTask> pTask = new TilingTask();
    pTask->m_nLevel = nLevel;
    pTask->m_nRow = nRow;
    pTask->m_nCol = nCol;

    //根据瓦片的Area找到所有被覆盖到的文件, FileList
    cmm::math::Box2d box2d;
    cmm::math::Point2d ptMin, ptMax;
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    if (pPyramid == NULL)
    {
        return false;
    }

    pPyramid->getTilePos(nLevel, nRow, nCol, ptMin.x(), ptMin.y(), ptMax.x(), ptMax.y());
    box2d.set(ptMin, ptMax);
    if (m_bHeightField)
    {
        extern const unsigned BYSIDE;
        if(BYSIDE != 0u)
        {
            const double dblSide = BYSIDE * 0.5;
            const double xPixel = dblSide * fabs(box2d.point0().x() - box2d.point1().x()) / m_nTileSize;
            const double yPixel = dblSide * fabs(box2d.point0().y() - box2d.point1().y()) / m_nTileSize;

            ptMin.x() -= xPixel;
            ptMin.y() -= yPixel;
            ptMax.x() += xPixel;
            ptMax.y() += yPixel;
            box2d.set(ptMin, ptMax);
        }
    }

    for (unsigned int i=0; i<m_vecSourceFile.size(); i++)
    {
        if (m_vecSourceFile[i]->isAreaInBoud(box2d))
        {
            pTask->m_vecSourceFiles.push_back(m_vecSourceFile[i]);
        }
    }

    m_pTaskQueue->addTask(pTask);
    m_pTilingThreadPool->wakeup();

    return true;
}

void TileBuilderImpl::writeConfigurationFile()
{
    bson::bsonDocument bsonDoc;
    ID instanceID = ID::genNewID();
    instanceID.ObjectID.m_nDataSetCode = m_nDataSetCode;
    if (!m_bHeightField)
    {
        instanceID.ObjectID.m_nType = TERRAIN_DOM_ID;
    }
    else
    {
        instanceID.ObjectID.m_nType = TERRAIN_DEM_ID;
    }

    bsonDoc.AddStringElement("ID", instanceID.toString().c_str());
    bsonDoc.AddStringElement("Name", m_strTargetDB.substr(m_strTargetDB.find_last_of('\\')+1, m_strTargetDB.length()).c_str());
    bsonDoc.AddInt64Element("UniqueID", m_pTaskQueue->getGlobeUniqueNumber());
    if (!m_bHeightField)
    {
        bsonDoc.AddInt32Element("Type", TERRAIN_DOM_ID);
    }
    else
    {
        bsonDoc.AddInt32Element("Type", TERRAIN_DEM_ID);
    }

    bson::bsonArrayEle *pBsonArray = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BoundingSphere"));
    pBsonArray->AddDblElement((m_TotalAreaBound.point0().x() + m_TotalAreaBound.point1().x()) / 2.0);
    pBsonArray->AddDblElement((m_TotalAreaBound.point0().y() + m_TotalAreaBound.point1().y()) / 2.0);

	osg::Vec3d ptBegin, ptEnd;
	osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(m_TotalAreaBound.point0().x(), m_TotalAreaBound.point0().y(), 0.0, ptBegin.x(), ptBegin.y(), ptBegin.z());
	osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(m_TotalAreaBound.point1().x(), m_TotalAreaBound.point1().y(), 0.0, ptEnd.x(), ptEnd.y(), ptEnd.z());
	osg::BoundingBox box(ptEnd, ptBegin);
	osg::BoundingSphere bs(box);

	pBsonArray->AddDblElement(0.0);
	//pBsonArray->AddDblElement(1000);
	pBsonArray->AddDblElement(bs.radius());

    pBsonArray = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("ChildrenID"));
    std::vector<ID> IDVec;
    m_pTaskQueue->getTopLevelID(IDVec);
    for (unsigned int i=0; i<IDVec.size(); i++)
    {
        bson::bsonDocumentEle *pBsonDocEle = dynamic_cast<bson::bsonDocumentEle *>(pBsonArray->AddDocumentElement());
        bson::bsonDocument *pChildBsonDoc = &pBsonDocEle->GetDoc();
        pChildBsonDoc->AddInt32Element(IDVec[i].toString().c_str(), m_nMaxLevel);
    }
    m_pTaskQueue->clearTopLevelID();

    std::string      strJsonString;
    bson::bsonStream bsonStream;
    bsonDoc.JsonString(strJsonString);
    bsonDoc.Write(&bsonStream);
    void*    pBuffer = bsonStream.Data();
    unsigned nBufLen = bsonStream.DataLen();

    m_pTargetDB->addBlock(instanceID, pBuffer, nBufLen);

    std::ofstream file;
    if (!m_bHeightField)
    {
        file.open(m_strTargetDB+".DeuDom", ios::out | ios::trunc);
    }
    else
    {
        file.open(m_strTargetDB+".DeuDem", ios::out | ios::trunc);
    }
    file<<"{\"IDList\":[\""<<instanceID.toString()<<"\"]}";
    file.close();

    return;
}

#include <Windows.h>
void TileBuilderImpl::getInvalidColor()
{
    m_vecInvalidColor.clear();

    char szAppPath[1024] = {0};
    ::GetModuleFileNameA(NULL, szAppPath, 1024);

    std::string strFileName = szAppPath;
    strFileName = strFileName.substr(0, strFileName.find_last_of('\\')) + "\\InvalidColor.ini";
    std::ifstream file;
    
    file.open(strFileName, ios::in);
    if (!file.is_open())
    {
        return ;
    }

    unsigned int nBegin = file.tellg();
    file.seekg (0, ios::end);
    unsigned int nEnd = file.tellg();
    unsigned int nLen = nEnd - nBegin;
    file.seekg (0, ios::beg);
    char* pbuf = new char[nLen + 1];
    memset(pbuf, 0, nLen + 1);
    file.read(pbuf, nLen);
    file.close();

    bson::bsonDocument bsonDoc;
    bsonDoc.FromJsonString(pbuf);
    delete [] pbuf;
    pbuf = NULL;

    std::string strTemp;
    bsonDoc.JsonString(strTemp);
    std::ofstream Ofile;
    Ofile.open(strFileName, ios::in | ios::out);
    if (!Ofile.is_open())
    {
        return;
    }
    /*Ofile.clear();*/
    Ofile<<strTemp;
    Ofile.close();

    bson::bsonStringEle* pUserInvalidColor = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("UseInvalidColor"));
    if (pUserInvalidColor == NULL)
    {
        return;
    }
    
    for (unsigned i = 0; i < bsonDoc.ChildCount(); i++)
    {
        std::string strName;
        std::string strValue;

        strName = bsonDoc.GetElement(i)->EName();
        if(strName != "InvalidCorlor") continue;

        bsonDoc.GetElement(i)->ValueString(strValue);
        unsigned nFirst = strValue.find_first_of(',');
        unsigned nLast  = strValue.find_last_of(',');
        if (nFirst == strValue.length() || nLast == strValue.length() || nLast <= nFirst) continue;

        unsigned int nR = atoi((strValue.substr(1, nFirst - 1)).c_str());
        unsigned int nG = atoi((strValue.substr(nFirst + 1, nLast - nFirst - 1)).c_str());
        unsigned int nB = atoi((strValue.substr(nLast + 1, strValue.length() - nLast - 1 - 1)).c_str());

        INVALIDCOLOR invalidColor;
        invalidColor.r = (char)nR;
        invalidColor.g = (char)nG;
        invalidColor.b = (char)nB;
        m_vecInvalidColor.push_back(invalidColor);
    }
}
