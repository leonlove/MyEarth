#include "DataSetManager.h"
#include "Service.h"
#include <comutil.h>
#include <AtlBase.h>
#include <sstream>
#include <algorithm>
#include "ServiceManager.h"
#include <Network/IDEUNetwork.h>
#include "DataKeeper.h"

namespace dk
{

    dk::IDataSetManager* CreateDataSetManager(void)
    {
        OpenSP::sp<IDataSetManager> pIDataSetManager = new DataSetManager();
        return pIDataSetManager.release();

    }


    DataSetManager::DataSetManager(void)
    {
        m_pDataKeeper = NULL;
        m_SetDataSetSrv.clear();
        m_SetDataSetLocal.clear();
    }


    DataSetManager::~DataSetManager(void)
    {
        m_SetDataSetSrv.clear();
        m_SetDataSetLocal.clear();
    }



    void DataSetManager::init()
    {
        this->update();
    }


    //本地数据的逻辑处理并提交后才能生效
    OpenSP::sp<IDataSet> DataSetManager::addLocalDataSet(const std::string &strDB)
    {
        if (strDB.empty())
        {
            return NULL;
        }

        OpenSP::sp<dk::IDataSet> pDstObj = dk::createDataSet();
        pDstObj->setDataSetManager(this);
        pDstObj->m_DataSource = LOCAL;

        return pDstObj;
    }


    bool DataSetManager::removeLocalDataSet(const std::string &strDB)
    {
        if (strDB.empty())
        {
            return true;
        }

        VECDATASET::iterator it = m_SetDataSetLocal.begin();
        for (; it != m_SetDataSetLocal.end(); ++it)
        {
            if((*it)->m_strFilePath == strDB)
            {
                (*it)->removeLocalFile();
                m_SetDataSetLocal.erase(it);

                return true;
            }
        }

        return true;
    }


    //本地数据的逻辑处理并提交后才能生效
    OpenSP::sp<IDataSet> DataSetManager::createLogicObj(const unsigned DataSetID, const std::string &strName)
    {
        bool isExist = false;
        VECDATASET::iterator it(m_SetDataSetSrv.begin());
        for(; it != m_SetDataSetSrv.end(); it++)
        {        
            if((*it)->m_DataSetID == DataSetID)
            {
                return *it;
            }
        }

        OpenSP::sp<dk::IDataSet> pDstObj = dk::createDataSet();
        pDstObj->setDataSetManager(this);
        pDstObj->m_DataSetID  = DataSetID;
        pDstObj->m_strName    = strName;
        pDstObj->m_DataSource = REMOTE;
        pDstObj->m_DataType   = NOTHING;

        m_SetDataSetSrv.insert(pDstObj);

        return pDstObj;
    }


    bool DataSetManager::removeLogicObj(const unsigned DataSetID)
    {
        VECDATASET::iterator itdst(m_SetDataSetSrv.begin());
        for(; itdst != m_SetDataSetSrv.end(); itdst++)
        {
            if((*itdst)->m_DataSetID == DataSetID)
            {
                std::set<OpenSP::sp<dk::IDataSetSegment>> pSegs            = (*itdst)->getDataSetSegments();
                std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator itSegs = pSegs.begin();
                for(; itSegs != pSegs.end(); ++itSegs)
                {
                    for(int k=0; k<(*itSegs)->getServicesCount(); k++)
                    {
                        OpenSP::sp<dk::IService> pIServiceObj = (*itSegs)->getService(k);
                        if(!pIServiceObj) continue;

                        pIServiceObj->delDataSetSegment(*itSegs);
                    }

                    (*itSegs)->deleteAllService();
                }

                m_SetDataSetSrv.erase(itdst);
                return true;
            }
        }

        return false;
    }



    unsigned DataSetManager::getDataSetCount(void) const
    {
        return (unsigned)m_SetDataSetSrv.size();
    }


    bool DataSetManager::deleteDataSet(unsigned nDataSetCode)
    {
        USES_CONVERSION;
        VECDATASET::iterator itdst(m_SetDataSetSrv.begin());
        for(; itdst != m_SetDataSetSrv.end(); itdst++)
        {
            if((*itdst)->m_DataSetID == nDataSetCode)
            {
                std::set<OpenSP::sp<dk::IDataSetSegment>> pSegs            = (*itdst)->getDataSetSegments();
                std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator itSegs = pSegs.begin();
                for(; itSegs != pSegs.end(); ++itSegs)
                {
                    for(int k=0; k<(*itSegs)->getServicesCount(); k++)
                    {
                        OpenSP::sp<dk::IService> pIServiceObj = (*itSegs)->getService(k);
                        if(!pIServiceObj) continue;

                        pIServiceObj->delDataSetSegment(*itSegs);

                        char szPort[48]={0};
                        itoa(pIServiceObj->getPort(), szPort, 10);

                        std::ostringstream oss;  BSTR bstrTmp;
                        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
                        if(pIUrlNetwork->deleteDataSet(nDataSetCode, pIServiceObj->getIP(), szPort))
                        {
                            oss<<pIServiceObj->getIP().c_str()<<":"<<szPort<<" code:"<<nDataSetCode<<" 数据删除成功\0";
                            bstrTmp = A2W(oss.str().c_str());
                        }
                        else                                    //Failed
                        {
                            oss<<pIServiceObj->getIP().c_str()<<":"<<szPort<<" code:"<<nDataSetCode<<" 数据删除失败\0";
                            bstrTmp = A2W(oss.str().c_str());
                        }
                    }

                    (*itSegs)->deleteAllService();
                }

                m_SetDataSetSrv.erase(itdst);
                break;
            }
        }

        return true;
    }


    bool DataSetManager::deleteDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)
    {
        if(!pServiceObj) return false;

        std::string strIP   = pServiceObj->getIP();
        char szPort[48]={0}; itoa(pServiceObj->getPort(), szPort, 10);

        char sz[48]={0};
        itoa(nCode, sz, 10); 
        std::string str; 
        str = strIP +":";
        str += szPort;
        str += " code:";
        str.append(sz);
        str += " ";

        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
        if(pIUrlNetwork->deleteDataSet(nCode, strIP, szPort))
        {
            str += " 数据删除成功";

            return true;

        }
        else
        {
            str += " 数据删除失败";

            return false;
        }

        return true;
    }


    OpenSP::sp<IDataSet> DataSetManager::getDataSetByCode(unsigned nCode)
    {
        USES_CONVERSION;

        VECDATASET::iterator itdst(m_SetDataSetSrv.begin());
        for(; itdst != m_SetDataSetSrv.end(); itdst++)
        {
            if((*itdst)->m_DataSetID == nCode)
            {
                return *itdst;
            }
        }

        return NULL;
    }

    OpenSP::sp<IDataSet> DataSetManager::getDataSetByIndex(unsigned nIndex)
    {
        USES_CONVERSION;

        unsigned nCounter = 0;
        VECDATASET::iterator itdst(m_SetDataSetSrv.begin());
        for(; itdst != m_SetDataSetSrv.end(); itdst++)
        {
            if(nIndex == nCounter)
            {
                return *itdst;
            }
            nCounter++;
        }

        return NULL;
    }

    OpenSP::sp<IDataSet> DataSetManager::getDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)
    {
        return pServiceObj->getDataSetByCode(nCode);
    }

    //同步数据到服务器上
    bool DataSetManager::commit(void)
    {
        //做个缓存容器
        std::map<unsigned int,RcdInfo> cacheDEUServerRoot;

        VECDATASET::iterator itdst(m_SetDataSetSrv.begin());
        for(; itdst != m_SetDataSetSrv.end(); ++itdst)
        {
            OpenSP::sp<dk::IDataSet> pDataset = *itdst;
            if(!pDataset) continue;

            RcdInfo _RcdInfo;
            _RcdInfo.m_strName = pDataset->m_strName;

            std::set<OpenSP::sp<dk::IDataSetSegment>> pSegs = pDataset->getDataSetSegments();

            std::set<std::string> _setIP; //过滤重复IP

            std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator itSegs = pSegs.begin();
            for(; itSegs != pSegs.end(); ++itSegs)
            {
                UrlInfo _UrlInfo;
                _UrlInfo.m_nStart = (*itSegs)->m_Min;
                _UrlInfo.m_nEnd   = (*itSegs)->m_Max;

                unsigned uSrvCount = (*itSegs)->getServicesCount();
                for(int i=0; i<uSrvCount; i++)
                {
                    OpenSP::sp<dk::IService> pService = (*itSegs)->getService(i);
                    _setIP.insert(pService->getIP());
                }

                //查找端口
                //std::vector<std::string> vecPort;
                std::set<std::string>::iterator it(_setIP.begin());
                for(; it != _setIP.end(); it++)
                {
                    std::vector<std::string> vecPort;
                    for(int p=0; p<uSrvCount; p++)
                    {
                        OpenSP::sp<dk::IService> pService = (*itSegs)->getService(p);
                        if(*it == pService->getIP()) //如果IP相等
                        {
                            char szPort[48]={0};
                            itoa(pService->getPort(), szPort, 10) ;

                            vecPort.push_back(szPort);
                        }
                    }

                    _UrlInfo.m_urlMap.insert(std::make_pair<std::string,std::vector<std::string>>(*it, vecPort));
                }

                _RcdInfo.m_urlInfoVec.push_back(_UrlInfo);
            }

            cacheDEUServerRoot.insert(std::make_pair<unsigned int,RcdInfo>(pDataset->m_DataSetID, _RcdInfo));
        }

        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        urlServer.removeAllUrl();
        std::map<unsigned int,RcdInfo>::iterator it(cacheDEUServerRoot.begin());
        for(; it != cacheDEUServerRoot.end(); it++)
        {
            urlServer.addUrlInfo(it->first, it->second);
        }

        urlServer.submit(&nError);

        cacheDEUServerRoot.clear();

        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
        pIUrlNetwork->initialize(strRootHost, strRootPort, true);

        return true;
    }


    //同步服务器上数据到本地
    void DataSetManager::update(void)
    {
        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        std::map<unsigned int,RcdInfo>::iterator it(rcdMap.begin());
        for(; it!=rcdMap.end() ;++it)
        {
            OpenSP::sp<dk::IDataSet> pDataSetObj = dk::createDataSet();
            pDataSetObj->setDataSetManager(this);
            pDataSetObj->m_strName   = it->second.m_strName.c_str();
            pDataSetObj->m_DataSetID = (ULONG)it->first;

            //处理数据集分区的
            unsigned int nURLTotal = it->second.m_urlInfoVec.size();
            std::vector<UrlInfo> vecUrl=it->second.m_urlInfoVec;
            for(int m=0; m<nURLTotal; m++)
            {
                OpenSP::sp<dk::IDataSetSegment> pDataSetSegment = pDataSetObj->findDataSetSegment(vecUrl[m].m_nStart, vecUrl[m].m_nEnd);
                if (pDataSetSegment != NULL)
                {
                    continue;
                }

                pDataSetSegment = dk::CreateDataSetSegment();
                pDataSetSegment->setParent(pDataSetObj);
                pDataSetSegment->m_Min = vecUrl[m].m_nStart;
                pDataSetSegment->m_Max = vecUrl[m].m_nEnd;

                std::map<std::string,std::vector<std::string>> hostMap = vecUrl[m].m_urlMap;
                std::map<std::string,std::vector<std::string>>::iterator itHostMap(hostMap.begin());
                for(; itHostMap != hostMap.end(); itHostMap++)
                {
                    std::vector<std::string> vecPort = itHostMap->second;
                    for(int nm=0; nm<vecPort.size(); nm++)
                    {
                        OpenSP::sp<dk::IServiceManager> pIServiceManager = m_pDataKeeper->getServiceManager();
                        OpenSP::sp<dk::IService> pServiceObj = pIServiceManager->findService(itHostMap->first.c_str(), atoi(vecPort[nm].c_str()));
                        if (pServiceObj == NULL)
                        {
                            pServiceObj = dk::createService();
                            pServiceObj->setServiceManager(pIServiceManager);

                            pServiceObj->setIP(itHostMap->first.c_str());
                            pServiceObj->setPort(atoi(vecPort[nm].c_str()));

                            pIServiceManager->addService(pServiceObj);
                        }

                        pServiceObj->addDataSetSegment(pDataSetSegment);

                        //增加服务到片段
                        pDataSetSegment->addService(pServiceObj);
                    }
                }
                pDataSetObj->addDataSetSegment(pDataSetSegment);
            }

            m_SetDataSetSrv.insert(pDataSetObj);
        }

        rcdMap.clear();
    }


    void DataSetManager::setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)
    {
        if (pDataKeeper != NULL)
        {
            m_pDataKeeper = pDataKeeper;
        }
    }

    OpenSP::sp<IDataKeeper> DataSetManager::getDataKeeper()
    {
        return m_pDataKeeper;
    }

}
