#include "DataSet.h"
#include <direct.h>
#include <Network/IDEUNetwork.h>
#include <IDProvider/Definer.h>
#include "DataKeeper.h"
#include "DEUDefine.h"
#include "DEULog.h"

#pragma warning (disable:4482)

namespace dk
{

    dk::IDataSet* createDataSet()
    {
        OpenSP::sp<dk::IDataSet> pDataSetObj = new DataSet;
        pDataSetObj->m_DataSource = REMOTE;
        return pDataSetObj.release();
    }


    DataSet::DataSet(void)
    {
        m_DataType       = NOTHING;
        m_pdeudb         = NULL;
        m_pDEULocalTile  = NULL;
        m_pDEULocalModel = NULL;
        m_pEventFeature  = NULL;
    }


    DataSet::~DataSet(void)
    {
        if (m_pdeudb)
        {
            m_pdeudb->closeDB();
            m_pdeudb = NULL;
        }

        m_DataType       = NOTHING;
        m_pDEULocalTile  = NULL;
        m_pDEULocalModel = NULL;
        m_pEventFeature  = NULL;
    }


    void DataSet::setEventFeatureID(ID* pID)
    {
        m_pEventFeature = pID;
    }


    bool DataSet::initLocalData(const std::string& sFilePath)
    {
        m_strFilePath = sFilePath;

        m_DataSource = LOCAL;

        if(!m_pdeudb)
            m_pdeudb = deudbProxy::createDEUDBProxy();

        if(m_pdeudb->openDB(m_strFilePath))
        {
            OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
            OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
            OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

            unsigned int nCount = m_pdeudb->getBlockCount();
            if(nCount < 0)
            {
                return false;
            }
            else if (nCount > 10)
            {
                nCount = 10;
            }

            std::vector<ID> vecIDs;
            m_pdeudb->getIndices(vecIDs, 0, nCount);

            int nDataType = 0;

            for (unsigned n=0; n<vecIDs.size(); n++)
            {
                if (vecIDs[n].m_nHighBit == 0 && vecIDs[n].m_nLowBit == 0 && vecIDs[n].m_nMidBit == 0)
                {
                    continue;
                }

                nDataType   = vecIDs[n].TileID.m_nType;
                m_DataSetID = vecIDs[n].TileID.m_nDataSetCode;
                break;
            }


            if (nDataType == TERRAIN_TILE_HEIGHT_FIELD  ||
                nDataType == TERRAIN_DEM_ID             ||
                nDataType == TERRAIN_DEM_LAYER_ID       ||  //地形
                nDataType == TERRAIN_TILE               ||
                nDataType == TERRAIN_TILE_IMAGE         ||
                nDataType == TERRAIN_DEM_ID             ||
                nDataType == TERRAIN_DOM_ID             ||//影像
                nDataType == TERRAIN_DOM_LAYER_ID) 
            {
                m_DataType = TILE;
                m_pDEULocalTile = new DEULocalTile(m_pEventFeature);
                m_pDEULocalTile->setDEUNetWork(pIUrlNetwork);
                m_pDEULocalTile->initialize(m_strFilePath);
            }
            else if (nDataType == IMAGE_ID                  || //矢量、模型
                     nDataType == MODEL_ID                  ||
                     nDataType == SHARE_MODEL_ID            ||
                     nDataType == CULTURE_LAYER_ID          ||
                     nDataType == PARAM_POINT_ID            ||
                     nDataType == PARAM_LINE_ID             ||
                     nDataType == PARAM_FACE_ID             ||
                     nDataType == SHARE_IMAGE_ID            ||
                     nDataType == DETAIL_PIPE_CONNECTOR_ID  ||
                     nDataType == DETAIL_CUBE_ID            ||
                     nDataType == DETAIL_CYLINDER_ID        ||
                     nDataType == DETAIL_PRISM_ID           ||
                     nDataType == DETAIL_PYRAMID_ID         ||
                     nDataType == DETAIL_SPHERE_ID          ||
                     nDataType == DETAIL_SECTOR_ID          ||
                     nDataType == DETAIL_STATIC_MODEL_ID    ||
                     nDataType == DETAIL_DYN_POINT_ID       ||
                     nDataType == DETAIL_DYN_LINE_ID        ||
                     nDataType == DETAIL_DYN_FACE_ID        ||
                     nDataType == DETAIL_DYN_IMAGE_ID       ||
                     nDataType == DETAIL_BUBBLE_TEXT_ID     ||
                     nDataType == DETAIL_POLYGON_ID         ||
                     nDataType == DETAIL_ROUND_TABLE_ID)
            {
                m_DataType = MODEL;
                m_pDEULocalModel = new DEULocalModel(m_pEventFeature);
                m_pDEULocalModel->setDEUNetWork(pIUrlNetwork);
                m_pDEULocalModel->initialize(m_strFilePath);
            }

            return true;
        }
    
        return false;

    }


    void DataSet::removeLocalFile()
    {
        if (m_pdeudb)
        {
            m_pdeudb->closeDB();
            m_pdeudb = NULL;
        }
    }


    unsigned DataSet::getDataSetSegmentCount()
    {
        return m_SetSegment.size();
    }


    bool DataSet::addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)
    {
        pDataSetSeg->setParent(this);

        m_SetSegment.insert(pDataSetSeg);

        return true;
    }


    OpenSP::sp<dk::IDataSetSegment> DataSet::getDataSetSegment(unsigned index)
    {
        unsigned n = 0;
        std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator it = m_SetSegment.begin();
        for(; it!=m_SetSegment.end(); ++it)
        {
            if(index == n)
            {
                return *it;
            }

            n++;
        }

        return NULL;
    }


    std::set<OpenSP::sp<dk::IDataSetSegment>>& DataSet::getDataSetSegments()
    {
        return m_SetSegment;
    }

    OpenSP::sp<dk::IDataSetSegment> DataSet::createDataSetSegment(void)
    {
        OpenSP::sp<dk::IDataSetSegment> _DataSetSegment = new DataSetSegment();
        return _DataSetSegment.release();
    }

    unsigned DataSet::getDataItemCount(void) 
    {

        if(m_DataSource == REMOTE)
        {
            //统计当前数据集数据总和，通过散列区间去汇总，覆盖单个区间数据统计的特例
            int Counter=0;
            OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
            OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
            OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

            std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator it = m_SetSegment.begin();
            for(; it != m_SetSegment.end(); it++)
            {
                unsigned uSrvCount = (*it)->getServicesCount();

                for(int m=0; m<uSrvCount; m++)
                {
                    OpenSP::sp<IService> pIServiceObj = (*it)->getService(m);
                    if(!pIServiceObj) continue;

                    std::string strIP; unsigned uPort=0;
                    strIP = pIServiceObj->getIP();
                    uPort = pIServiceObj->getPort();

                    char szPort[48]={0};
                    itoa(uPort,szPort,10);

                    Counter += pIUrlNetwork->queryBlockCount(strIP, szPort, m_DataSetID);
                }
            }

            return Counter;
        }
        else if(m_DataSource == LOCAL)
        {
            //似乎不太稳妥
            if(m_pdeudb) 
                return (unsigned)m_pdeudb->getBlockCount();
            else
                return -1;
        }

        return -1;
    }

    void DataSet::setDataSetManager(OpenSP::sp<IDataSetManager> pDataSetManager)
    {
        m_pDataSetManager = pDataSetManager;
    }

    OpenSP::sp<IDataSetManager> DataSet::getDataSetManager()
    {
        return m_pDataSetManager;
    }

    OpenSP::sp<dk::IDataSetSegment> DataSet::findDataSetSegment(unsigned nMin, unsigned nMax)
    {
        std::set<OpenSP::sp<dk::IDataSetSegment> >::iterator itr = m_SetSegment.begin();
        for (; itr!=m_SetSegment.end(); itr++)
        {
            if ((*itr)->m_Min==nMin && (*itr)->m_Max==nMax)
            {
                return *itr;
            }
        }

        return NULL;
    }

    ID DataSet::getDataItem(unsigned nIndex)
    {
        //远程服务器下载的数据
        if(m_DataSource == REMOTE)
        {
            int Counter=0;
            OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
            OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
            OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

            //遍历散列分布区间
            std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator it = m_SetSegment.begin();
            for(; it != m_SetSegment.end(); it++)
            {
                unsigned uSrvCount = (*it)->getServicesCount(); //区间下服务
                for(int m=0; m<uSrvCount; m++)
                {
                    OpenSP::sp<dk::IService> _IServiceObj = (*it)->getService(m);

                    std::string strIP; unsigned uPort=0;
                    strIP = _IServiceObj->getIP();
                    uPort = _IServiceObj->getPort();

                    //查找对应服务下的数据
                    char szPort[48]={0};
                    itoa(uPort,szPort,10);

                    Counter += pIUrlNetwork->queryBlockCount(strIP, szPort, m_DataSetID);
                    if(nIndex < Counter)
                    {
                        nIndex -= (Counter - pIUrlNetwork->queryBlockCount(strIP, szPort, m_DataSetID)); //计算偏移量

                        std::vector<ID> idVec;
                        pIUrlNetwork->queryIndices(strIP, szPort, m_DataSetID, nIndex, 1, idVec);

                        return idVec[0];
                    }
                }
            }
        }
        else if(m_DataSource == LOCAL) //本地加载的数据的总数
        {
            if (m_pdeudb == NULL)
            {
                ID Id;
                return Id;
            }

            if(nIndex < m_pdeudb->getBlockCount())
            {
                std::vector<ID> VecID;
                m_pdeudb->getIndices(VecID, nIndex, 1);
                if(VecID.size() > 0){
                    std::string strID = VecID[0].toString().c_str();
                    ID id = ID::genIDfromString(strID);
                    return id;
                }
            }
        }

        ID Id;
        return Id;
    }


    bool DataSet::getIndicesData(const std::string& strHost, const std::string& strPort, unsigned nDSCode, unsigned nOffset,unsigned nCount, std::vector<ID>& vecIds)
    {
        OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
        OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

        return pIUrlNetwork->queryIndices(strHost, strPort, nDSCode, nOffset, nCount, vecIds);
    }

    void DataSet::DelRemoteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const std::vector<ID>& IDCollect)
    {
        std::vector<ID> errVec;
        OpenSP::sp<cmm::IDEUException> pOutExcep = cmm::createDEUException();
        if (pOutExcep == NULL)
        {
            return;
        }

        OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
        OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();
        if(!pIUrlNetwork->deleteData(strHost, strPort, IDCollect, errVec, pOutExcep))
        {
            //异常的日志输出
        }

        if (!pIUrlNetwork->deleteLayerChildren(IDCollect, pOutExcep))
        {
            //异常的日志输出
        }
    }

    bool DataSet::deleteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const IDList &listIDs)
    {
        DelRemoteData(strHost, strPort, pEventAdapter, listIDs);

        return true; 
    }


    bool DataSet::isDataItemExist(const ID &id, const std::string& strIP, const std::string& strPort)
    { 
        if(m_DataSource == REMOTE)
        {
            int Counter = 0;
            OpenSP::sp<IDataSetManager> pIDataSetManager = getDataSetManager();
            OpenSP::sp<IDataKeeper>  pIDataKeeper = pIDataSetManager->getDataKeeper();
            OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

            //遍历散列分布区间
            std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator it = m_SetSegment.begin();
            for(; it != m_SetSegment.end(); it++)
            {
                unsigned uSrvCount = (*it)->getServicesCount(); //区间下服务
                for(int m=0; m<uSrvCount; m++)
                {
                    OpenSP::sp<dk::IService> _IServiceObj = (*it)->getService(m);

                    std::string strip;
                    unsigned    uPort=0;
                    strip = _IServiceObj->getIP();
                    uPort = _IServiceObj->getPort();

                    if (strIP != strip || uPort != atoi(strPort.c_str()))
                    {
                        continue;
                    }

                    //查找对应服务下的数据
                    char szPort[48]={0};
                    itoa(uPort,szPort,10);

                    Counter = pIUrlNetwork->queryBlockCount(strIP, szPort, m_DataSetID);
                    for(int n=0; n<Counter; n++)
                    {
                        std::vector<ID> idVec;
                        pIUrlNetwork->queryIndices(strIP, szPort, m_DataSetID, n, 1, idVec);

                        if(id == idVec[0])
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }
        else if(m_DataSource == LOCAL)
        {
            if(m_pdeudb)
            {
                return m_pdeudb->isExist(id);
            }
        }

        return false; 
    }

    //局部提交操作
    //返回值：0 - 成功；1 - 终止；2 - 失败
    unsigned DataSet::commitSection(ea::IEventAdapter* pEventAdapter, const IDList &listIDs)
    {
        if(m_DataSource == REMOTE)
        {
            return 2;
        }
        else if(m_DataType == TILE && m_pDEULocalTile != NULL)
        {
            return m_pDEULocalTile->submit(pEventAdapter, (std::vector<ID>)listIDs);
        }
        else if(m_DataType == MODEL && m_pDEULocalModel != NULL)
        {
            return m_pDEULocalModel->submit(pEventAdapter, (std::vector<ID>)listIDs);
        }

        return 2;
    }


    //全部提交操作
    //返回值：0 - 成功；1 - 终止；2 - 失败
    unsigned DataSet::commit(ea::IEventAdapter* pEventAdapter)
    {
        OpenSP::sp<LogInfo> pCurLogInfo = new LogInfo;

        if(m_DataSource == REMOTE)
        {
            return 2;
        }
        else if(m_DataType == TILE && m_pDEULocalTile != NULL)
        {
            m_pDEULocalTile->setLogInfo(pCurLogInfo);
            m_pDEULocalTile->setBatchUpload(getDataSetManager()->getDataKeeper()->getBatchUpload());
            unsigned nRet = m_pDEULocalTile->submit(pEventAdapter, std::vector<ID>());
            CDeuLog::getDefaultInstance()->WriteLogInfo(pCurLogInfo);
            
            return nRet;
        }
        else if(m_DataType == MODEL && m_pDEULocalModel != NULL)
        {
            m_pDEULocalModel->setLogInfo(pCurLogInfo);
            m_pDEULocalModel->setBatchUpload(getDataSetManager()->getDataKeeper()->getBatchUpload());
            unsigned nRet = m_pDEULocalModel->submit(pEventAdapter, std::vector<ID>());
            CDeuLog::getDefaultInstance()->WriteLogInfo(pCurLogInfo);

            return nRet;
        }

        //CDeuLog::getDefaultInstance()->WriteLogInfo(pCurLogInfo);

        return 2; 
    }


    void DataSet::commitBreak(ea::IEventAdapter* pEventAdapter, bool isFinish)
    {
        if(m_DataType == TILE && m_pDEULocalTile != NULL)
        {
            m_pDEULocalTile->m_bIsFinish = isFinish;
            if(isFinish)
            {
                CDeuCallEvent::CallBreakEventState("提交已中止", m_pEventFeature, pEventAdapter);
            }
        }
        else if(m_DataType == MODEL && m_pDEULocalModel != NULL)
        {
            m_pDEULocalModel->m_bIsFinish = isFinish;
            if(isFinish)
            {
                CDeuCallEvent::CallBreakEventState("提交已中止", m_pEventFeature, pEventAdapter);
            }
        }
    }


    void DataSet::refresh(void)
    {

    }

}
