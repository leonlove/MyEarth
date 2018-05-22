#include "Service.h"
#include "DEUUrlServer.h"
#include "DataSet.h"
#include "Network/IDEUNetwork.h"
#include "DataKeeper.h"
#include <sstream>

namespace dk
{

    dk::IService* createService()
    {
        OpenSP::sp<IService> pServiceObj = new Service;

        return pServiceObj.release();
    }


    Service::Service(void)
        :m_bState(false)
        ,m_strIP("")
        ,m_nPort(0)
        ,m_pEventFeature(NULL)
    {
    }


    Service::~Service(void)
    {
    }


    void Service::setEventFeatureID(ID* pID)
    {
        m_pEventFeature = pID;
    }


    bool Service::start(ea::IEventAdapter* p_EventAdapter)
    {
        std::ostringstream ossPort;
        ossPort<<m_nPort;

        OpenSP::sp<IServiceManager> pIServiceManager = getServiceManager();
        OpenSP::sp<IDataKeeper> pIDataKeeper = pIServiceManager->getDataKeeper();
        //std::string strRootPort = pIDataKeeper->getRootPort();
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

        string strApache = pIServiceManager->findApache(m_strIP);
        bool bRet = pIUrlNetwork->startService(m_strIP, /*strRootPort*/strApache, ossPort.str(), "start");

        if (p_EventAdapter == NULL)
        {
            return bRet;
        }
        bRet = this->isRunning();
        if (!bRet)
        {
            OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
            pEventObj->setAction(ea::ACTION_START_SERVICE);

            std::ostringstream ossErr;
            ossErr<<m_strIP.c_str()<<":"<<m_nPort<<" 启动服务操作失败\0";
            pEventObj->putExtra("Descrip", ossErr.str());

            std::string strID = "";
            if (m_pEventFeature)
            {
                strID = m_pEventFeature->toString();
            }
            pEventObj->putExtra("Pointer", strID);

            p_EventAdapter->sendBroadcast(pEventObj);
        }
        else
        {
            OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();

            pEventObj->setAction(ea::ACTION_START_SERVICE);
            std::ostringstream oss;
            oss<<m_strIP.c_str()<<":"<<m_nPort<<" 启动服务操作已执行\0";

            m_bState = true;

            pEventObj->putExtra("Descrip", oss.str());

            std::string strID = "";
            if (m_pEventFeature)
            {
                strID = m_pEventFeature->toString();
            }
            pEventObj->putExtra("Pointer", strID);

            p_EventAdapter->sendBroadcast(pEventObj);
        }

        return bRet;
    }


    bool Service::stop(ea::IEventAdapter* p_EventAdapter)
    {
        std::ostringstream ossPort;
        ossPort<<m_nPort;

        OpenSP::sp<IServiceManager> pIServiceManager = getServiceManager();
        OpenSP::sp<IDataKeeper> pIDataKeeper = pIServiceManager->getDataKeeper();
        //std::string strRootPort = pIDataKeeper->getRootPort();
        OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork = pIDataKeeper->getDEUNetWork();

        string strApache = pIServiceManager->findApache(m_strIP);
        if (!pIUrlNetwork->startService(m_strIP, /*strRootPort*/strApache, ossPort.str(), "stop"))
        {
            OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
            pEventObj->setAction(ea::ACTION_STOP_SERVICE);

            std::ostringstream ossErr;
            ossErr<<m_strIP.c_str()<<":"<<m_nPort<<" 停止服务操作失败\0";
            pEventObj->putExtra("Descrip", ossErr.str());

            std::string strID = "";
            if (m_pEventFeature)
            {
                strID = m_pEventFeature->toString();
            }
            pEventObj->putExtra("Pointer", strID);

            p_EventAdapter->sendBroadcast(pEventObj);
            return false;
        }
        else
        {
            OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
            pEventObj->setAction(ea::ACTION_STOP_SERVICE);

            std::ostringstream ossErr;
            ossErr<<m_strIP.c_str()<<":"<<m_nPort<<" 停止服务操作已执行\0";
            pEventObj->putExtra("Descrip", ossErr.str());

            std::string strID = "";
            if (m_pEventFeature)
            {
                strID = m_pEventFeature->toString();
            }
            pEventObj->putExtra("Pointer", strID);

            p_EventAdapter->sendBroadcast(pEventObj);

            m_bState = false;
            return true;
        }

        return true;
    }


    const std::string &Service::getIP(void) const
    {
        return m_strIP;
    }


    void Service::setIP(const std::string &strIP)
    {
        this->m_strIP = strIP;
    }


    unsigned Service::getPort(void) const
    {
        return m_nPort;
    }


    void Service::setPort(unsigned nPort)
    {
        this->m_nPort = nPort;
    }


    bool Service::isRunning(void)
    {
        OpenSP::sp<IServiceManager> pServiceManager = getServiceManager();
        OpenSP::sp<deunw::IDEUNetwork> pDEUNetwork = pServiceManager->getDataKeeper()->getDEUNetWork();

        char buffer[10]={0};
        _itoa_s(m_nPort, buffer, 10);
        std::string strPort(buffer);
        std::string strApache = pServiceManager->findApache(m_strIP);

        m_bState = pDEUNetwork->checkPortIsActive(m_strIP, strApache, strPort);
        return m_bState;
    }


    bool Service::addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)
    {
        if (pDataSetSeg != NULL)
        {
            m_SetSegment.insert(pDataSetSeg);
        }

        return true;
    }


    bool Service::delDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)
    {
        if (pDataSetSeg != NULL)
        {
            m_SetSegment.erase(pDataSetSeg);
        }

        return true;
    }


    OpenSP::sp<IDataSet> Service::getDataSetByCode(unsigned nDataSetCode)
    {
        std::set<OpenSP::sp<dk::IDataSetSegment>>::iterator it = m_SetSegment.begin();
        for (; it!=m_SetSegment.end(); ++it)
        {
            OpenSP::sp<IDataSet> pDataSetObj = (*it)->getParent();
            if (pDataSetObj != NULL && pDataSetObj->m_DataSetID == nDataSetCode)
            {
                return pDataSetObj;
            }
        }

        return NULL;
    }


    unsigned Service::getDataSetSegmentCount()
    {
        return m_SetSegment.size();
    }


    OpenSP::sp<dk::IDataSetSegment> Service::getDataSetSegment(unsigned index)
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


    std::set<OpenSP::sp<dk::IDataSetSegment>>& Service::getDataSetSegments()
    {
        return m_SetSegment;
    }

    OpenSP::sp<dk::IDataSetSegment> Service::createDataSetSegment(void)
    {
        OpenSP::sp<dk::IDataSetSegment> _DataSetSegment = new DataSetSegment();
        return _DataSetSegment.release();
    }

    void Service::setServiceManager(OpenSP::sp<IServiceManager> pServiceManager)
    {
        m_pServiceManager = pServiceManager;
    }

    OpenSP::sp<IServiceManager> Service::getServiceManager()
    {
        return m_pServiceManager;
    }

}

