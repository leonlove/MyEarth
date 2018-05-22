#include "ServiceManager.h"
#include "DataSetManager.h"
#include "DEUUrlServer.h"
#include "export.h"
#include "Network/DEUNetwork.h"
#include "DataKeeper.h"
#include <algorithm>

namespace dk
{

    dk::IServiceManager* CreateServiceManager(void)
    {
        OpenSP::sp<dk::IServiceManager> pServiceManager = new ServiceManager();
        return pServiceManager.release();
    }


    ServiceManager::ServiceManager()
    {
        m_pDataKeeper = NULL;
        m_SetServices.clear();
    }


    ServiceManager::~ServiceManager()
    {

    }


    void ServiceManager::init()
    {
        this->update();
    }


    bool ServiceManager::addService(const std::string &strIP, unsigned nPort, OpenSP::sp<dk::IService>& pServiceObj)
    {
        bool isExist = false;

        std::set<OpenSP::sp<dk::IService>>::iterator it = m_SetServices.begin();
        for(; it!=m_SetServices.end(); ++it)
        {
            std::string sIP = (*it)->getIP();
            unsigned uPort  = (*it)->getPort();

            if((strIP == sIP) && (nPort == uPort))
            {
                pServiceObj = *it;
                isExist = true;
                break;
            }
        }

        if(false == isExist)
        {
            pServiceObj = dk::createService();
            pServiceObj->setServiceManager(this);

            pServiceObj->setIP(strIP);
            pServiceObj->setPort(nPort);
            m_SetServices.insert(pServiceObj);
        }

        return true;
    };


    bool ServiceManager::addService(OpenSP::sp<dk::IService> pServiceObj)
    {
        std::set<OpenSP::sp<dk::IService>>::iterator it = m_SetServices.begin();
        for(; it!=m_SetServices.end(); ++it)
        {
            if((pServiceObj->getIP() == (*it)->getIP()) && (pServiceObj->getPort() == (*it)->getPort()))
            {
                return false;
            }
        }

        m_SetServices.insert(pServiceObj);
        return true;
    }


    unsigned ServiceManager::getServiceCount(void) const
    {
        return (unsigned)m_SetServices.size();
    };


    OpenSP::sp<IService> ServiceManager::getService(unsigned nIndex)
    {
        unsigned n = 0;
        std::set<OpenSP::sp<dk::IService>>::iterator it = m_SetServices.begin();
        for(; it!=m_SetServices.end(); ++it)
        {
            if(nIndex == n)
            {
                return *it;
            }

            n++;
        }

        return NULL;
    };


    bool ServiceManager::delService(const std::string &strIP, unsigned nPort)
    {
        std::set<OpenSP::sp<dk::IService>>::iterator it(m_SetServices.begin());
        for(; it != m_SetServices.end(); it++)
        {
            std::string sIP = (*it)->getIP(); 
            unsigned uPort  = (*it)->getPort();

            if((strIP == sIP) && (nPort == uPort))
            {
                m_SetServices.erase(it);
                return true;
            }
        }

        return false;
    }


    OpenSP::sp<IService> ServiceManager::findService(const std::string &strIP, unsigned nPort)
    {
        unsigned n = 0;
        std::set<OpenSP::sp<dk::IService>>::iterator it = m_SetServices.begin();
        for(; it!=m_SetServices.end(); ++it)
        {
            std::string sIP = (*it)->getIP();
            unsigned uPort  = (*it)->getPort();

            if((strIP == sIP) && (nPort == uPort))
            {
                return *it;
            }

            n++;
        }

        return NULL;
    };


    bool ServiceManager::findServiceByCode(unsigned nCode, cmm::variant_data &varServiceList)
    {
        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        std::vector<std::string>    vecServiceList;
        int nError=0;
        std::map<unsigned int,RcdInfo> maps;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(maps, hostMap, &nError);

        std::map<unsigned int,RcdInfo>::iterator it(maps.begin());
        for(; it != maps.end(); it++)
        {
            if(it->first == nCode)
            {
                std::vector<UrlInfo> url = it->second.m_urlInfoVec;
                for(int p=0; p<url.size();p++)
                {
                    std::map<std::string,std::vector<std::string>> mapsHost = url[p].m_urlMap;
                    std::map<std::string,std::vector<std::string>>::iterator itHost(mapsHost.begin());
                    for(; itHost != mapsHost.end(); itHost++)
                    {
                        std::vector<std::string> vecPort = itHost->second;
                        for(int nm=0; nm<vecPort.size(); nm++)
                        {
                            std::string strHost = itHost->first;
                            strHost += ":";
                            strHost += vecPort[nm].c_str();

                            vecServiceList.push_back(strHost);  //IP：Port
                        }
                    }
                }

                varServiceList = vecServiceList;
                return true;
            }
        }

        return false;
    };


    bool ServiceManager::start(void)
    {
        std::set<OpenSP::sp<dk::IService>>::iterator itsrc = m_SetServices.begin();
        for(; itsrc!=m_SetServices.end(); ++itsrc)
        {
            (*itsrc)->start(NULL);
        }

        return true; 
    };


    bool ServiceManager::stop(void)
    { 
        std::set<OpenSP::sp<dk::IService>>::iterator itsrc = m_SetServices.begin();
        for(; itsrc!=m_SetServices.end(); ++itsrc)
        {
            (*itsrc)->stop(NULL);
        }

        return true; 
    };


    //下行
    bool ServiceManager::update(void)
    { 
        //获取散列信息
        m_mapHost.clear();
        m_mapApacheHost.clear();

        int nError;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;

        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        urlServer.getInfo(rcdMap, hostMap, &nError);

        bool isExist = false;
        std::map<std::string,HostInfo>::iterator it = hostMap.begin();
        for (; it!=hostMap.end(); ++it)
        {
            m_mapApacheHost[it->first] = it->second.m_strApachePort;

            std::vector<std::string> vecPort = it->second.m_portVec;
            for (int i=0; i<vecPort.size(); i++)
            {
                isExist = false;

                std::set<OpenSP::sp<dk::IService>>::iterator itsrc = m_SetServices.begin();
                for(; itsrc!=m_SetServices.end(); ++itsrc)
                {
                    if(((*itsrc)->getIP() == it->first) && ((*itsrc)->getPort() == atoi(vecPort[i].c_str())))
                    {
                        isExist = true;
                        break;
                    }
                }

                if(isExist == false)
                {
                    OpenSP::sp<dk::IService> pServiceObj = dk::createService();
                    pServiceObj->setServiceManager(this);

                    pServiceObj->setIP(it->first.c_str());
                    pServiceObj->setPort(atoi(vecPort[i].c_str()));
                    m_SetServices.insert(pServiceObj);
                }
            }
        }

        return true; 
    };


    //上行
    bool ServiceManager::commit(void)
    { 
        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        urlServer.removeAllDataServer();

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        std::string strIPTmp, strPortTmp;
        std::map<std::string, std::vector<std::string>>::iterator itIP(m_mapHost.begin());

        //-------------------------------------------------------------------------------------
        //- 合并url
        std::map<std::string,HostInfo> hostMapImport;
        getDataKeeper()->getImportUrl(hostMapImport);
        std::map<std::string,HostInfo>::iterator itrImportHost = hostMapImport.begin();
        for (; itrImportHost!=hostMapImport.end(); itrImportHost++)
        {
            std::map<std::string,HostInfo>::iterator itrHostTmp = hostMap.find(itrImportHost->first);
            if (itrHostTmp == hostMap.end())
            {
                hostMap.insert(*itrImportHost);
            }
            else
            {
                for (int i = 0; i < itrImportHost->second.m_portVec.size(); i++)
                {
                    if (std::find(itrHostTmp->second.m_portVec.begin(), itrHostTmp->second.m_portVec.end(), itrImportHost->second.m_portVec[i]) == itrHostTmp->second.m_portVec.end())
                        itrHostTmp->second.m_portVec.push_back(itrImportHost->second.m_portVec[i]);
                }
            }
        }

        itrImportHost = hostMap.begin();
        urlServer.removeAllDataServer();
        m_mapApacheHost.clear();
        for (; itrImportHost!=hostMap.end(); itrImportHost++)
        {
            //- 维护本地的m_mapApacheHost
            m_mapApacheHost[itrImportHost->first] = itrImportHost->second.m_strApachePort;

            if (!urlServer.addDataServer(itrImportHost->first, itrImportHost->second))
            {
                continue;
            }
        }

        //- 维护本地的m_SetServices
        itrImportHost = hostMap.begin();
        for (; itrImportHost!=hostMap.end(); ++itrImportHost)
        {
            m_mapApacheHost[itrImportHost->first] = itrImportHost->second.m_strApachePort;

            std::vector<std::string> vecPort = itrImportHost->second.m_portVec;
            for (int i=0; i<vecPort.size(); i++)
            {
                bool isExist = false;

                std::set<OpenSP::sp<dk::IService>>::iterator itsrc = m_SetServices.begin();
                for(; itsrc!=m_SetServices.end(); ++itsrc)
                {
                    if(((*itsrc)->getIP() == itrImportHost->first) && ((*itsrc)->getPort() == atoi(vecPort[i].c_str())))
                    {
                        isExist = true;
                        break;
                    }
                }

                if(isExist == false)
                {
                    OpenSP::sp<dk::IService> pServiceObj = dk::createService();
                    pServiceObj->setServiceManager(this);

                    pServiceObj->setIP(itrImportHost->first.c_str());
                    pServiceObj->setPort(atoi(vecPort[i].c_str()));
                    m_SetServices.insert(pServiceObj);
                }
            }
        }
        //-------------------------------------------------------------------------------------

        //循环查IP
        for(; itIP != m_mapHost.end(); itIP++)
        {
            HostInfo _hostinfo;
            _hostinfo.m_portVec.assign(itIP->second.begin(), itIP->second.end());
            _hostinfo.m_strApachePort = m_mapApacheHost.at(itIP->first);
            urlServer.addDataServer(itIP->first, _hostinfo);
        }

        nError = 0;
        urlServer.submit(&nError);

        return false; 
    };


    void ServiceManager::stateInfo(std::vector<std::string>& vecServiceState)
    {
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
        std::map<std::string, std::string>::iterator it = m_mapApacheHost.begin();
        for (; it!=m_mapApacheHost.end(); ++it)
        {
            std::vector<std::string> vecPort;
            pIUrlNetwork->getActivePorts(it->first, it->second, vecPort);

            for(int m=0; m<vecPort.size(); m++)
            {
                std::string sHost = it->first;
                sHost += ":";
                sHost += vecPort[m];
                vecServiceState.push_back(sHost);
            }
        }
    }


    bool ServiceManager::addServer(std::string& szHost, std::string& ApachePort, std::string& port)
    {
        int nError = 1;
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
//         if(!pIUrlNetwork->startService(szHost, ApachePort, port,"add"))
//         {
//             return false;
//         }
        if (!pIUrlNetwork->addServer(szHost, ApachePort))
        {
            return false;
        }
        
        m_mapApacheHost[szHost] = ApachePort;
        if (port != "")
        {
            OpenSP::sp<dk::IService> pServiceObj = NULL;
            addService(szHost, atoi(port.c_str()), pServiceObj);
        }

        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        bool isNotFind = true;
        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            if((strcmp(itr->first.c_str(), szHost.c_str()) == 0) &&
               (strcmp(itr->second.m_strApachePort.c_str(), ApachePort.c_str()) == 0))
            {
                for(int k=0; k<itr->second.m_portVec.size(); k++)
                {
                    if(strcmp(itr->second.m_portVec[k].c_str(), port.c_str()) == 0)
                    {
                        isNotFind = false;
                    }
                }
                if(isNotFind)
                {
                    //仅端口不存在的追加
                    itr->second.m_portVec.push_back(port);
                    urlServer.addDataServer(szHost, itr->second);
                    urlServer.submit(&nError);
                    return true;
                }
            }
        }

        //如果进入，则表示IP和端口都不存在
        if(isNotFind)
        {
            HostInfo _hostinfo;
            _hostinfo.m_strApachePort = ApachePort;
            _hostinfo.m_portVec.push_back(port);
            urlServer.addDataServer(szHost, _hostinfo);
            urlServer.submit(&nError);
            return true;
        }

        return true;
    }


    bool ServiceManager::delServer(std::string& szHost, std::string& ApachePort, std::string& port)
    {
        int nError = 1;

        /*if (port == "")
        {
            OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = m_pDataKeeper->getDEUNetWork();
            if (!pIUrlNetwork->delServer(szHost, NULL))
            {
                return false;
            }

            std::map<std::string, std::string>::iterator itr = m_mapApacheHost.begin();
            for (; itr!=m_mapApacheHost.end(); itr++)
            {
                if (itr->first == szHost)
                {
                    m_mapApacheHost.erase(itr);
                    break;
                }
            }
            return true;
        }*/

        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        bool isNotFind = false;
        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            if((strcmp(itr->first.c_str(), szHost.c_str()) == 0) &&
               (strcmp(itr->second.m_strApachePort.c_str(), ApachePort.c_str()) == 0))
            {
                std::vector<std::string>::iterator itVec = itr->second.m_portVec.begin();
                for(; itVec != itr->second.m_portVec.end(); itVec++)
                {
                    if(strcmp(itVec->c_str(), port.c_str()) == 0)
                    {
                        std::string strTmpp = itVec->c_str();
                        unsigned int Tmp_port = atoi(strTmpp.c_str());
                        delService(itr->first,Tmp_port);
                        itr->second.m_portVec.erase(itVec);
                        urlServer.addDataServer(itr->first, itr->second);
                        isNotFind = true;
                        break;
                    }
                }
                if(isNotFind)
                {
                    urlServer.submit(&nError);
                    return true;
                }
            }
        }

        return true;
    }


    void ServiceManager::getServer(std::vector<std::string>& vecSrvHost)
    {
        std::string strRootHost = m_pDataKeeper->getRootHost();
        std::string strRootPort = m_pDataKeeper->getRootPort();

        DEUUrlServer urlServer(m_pDataKeeper->getDEUNetWork());
        urlServer.initialize(strRootHost, strRootPort, false);

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            std::string strSrvHost = "";
            if (itr->second.m_portVec.size() != 0)
            {
                for(int k=0; k<itr->second.m_portVec.size(); k++)
                {
                    strSrvHost = itr->first.c_str();
                    strSrvHost+= ":" + itr->second.m_strApachePort + ":";
                    strSrvHost+= itr->second.m_portVec[k].c_str();
                    vecSrvHost.push_back(strSrvHost);
                }
            }
            else
            {
                strSrvHost = itr->first.c_str();
                strSrvHost+= ":" + itr->second.m_strApachePort + ":";
                strSrvHost+= "";
                vecSrvHost.push_back(strSrvHost);
            }
        }

        return;
    }

    OpenSP::sp<dk::IService> ServiceManager::createService(void)
    {
        OpenSP::sp<dk::IService> pServiceObj = dk::createService();
        return pServiceObj.release();
    }

    void ServiceManager::setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)
    {
        if (pDataKeeper != NULL)
        {
            m_pDataKeeper = pDataKeeper;
        }
    }

    OpenSP::sp<IDataKeeper> ServiceManager::getDataKeeper()
    {
        return m_pDataKeeper;
    }

    std::string ServiceManager::findApache(const std::string& strIP)
    {
        std::string strApache("");
        std::map<std::string, std::string>::iterator itr = m_mapApacheHost.begin();
        for (; itr!=m_mapApacheHost.end(); itr++)
        {
            if (itr->first == strIP)
            {
                strApache = itr->second;
                break;
            }
        }

        return strApache;
    }

}



