#include "WFSBuilder.h"
#include <common/Pyramid.h>
#include <EventAdapter/IEventFilter.h>
#include <EventAdapter/IEventObject.h>

#define   MAX_LEVEL   15
#define   THREADCOUNT 3

namespace wfsb
{

    WFSBuilder::WFSBuilder(void)
    {
        m_pDriver = NULL;
        m_pLayerManager = NULL;
        m_nThread = 0;
        m_pModelBuilder = NULL;
        m_pConvert = NULL;

        m_bInitialized = false;
    }

    WFSBuilder::~WFSBuilder(void)
    {
        if(m_bInitialized)
        {
            unInitialize();
        }
    }

    IWFSBuilder* createWFSBuilder()
    {
        OpenSP::sp<WFSBuilder> pBuilder = new WFSBuilder;
        return pBuilder.release();
    }

    bool WFSBuilder::initialize(deues::IWFSDriver*    pDriver,
        logical::ILayerManager* pLayerManager)
    {
        if(m_bInitialized)  return true;
        m_pDriver = pDriver;
        m_pLayerManager = pLayerManager;
        if(!createLogicalLayer())
        {
            return false;
        }
        unsigned nDSCode = m_pDriver->getDataSetCode();
        m_pModelBuilder = createModelBuilder();
        m_pModelBuilder->initialize("",nDSCode,NULL);

        m_pConvert = new ConvertGML();
        m_pConvert->initialize(m_pModelBuilder,nDSCode,pDriver->getVersion(),pDriver->getUrl());
        
        std::vector<FEATURELAYERMAP> mapVec;
        mapVec.resize(THREADCOUNT);

        FEATURELAYERMAP::const_iterator pItr = m_mapFeatureLayer.cbegin();
        unsigned nTemp = 0;
        while(pItr != m_mapFeatureLayer.cend())
        {
            mapVec[nTemp%THREADCOUNT][pItr->first] = pItr->second;
            nTemp++;
            pItr++;
        }
        
        for(unsigned n = 0;n < mapVec.size();n++)
        {
            if(!mapVec[n].empty())
            {
                BuilderThread* pThread = new BuilderThread(this,mapVec[n]);
                pThread->startThread();
                m_vecBuildingThread.push_back(pThread);
            }
        }

        m_bInitialized = true;
        return true;
    }

    void WFSBuilder::unInitialize()
    {
        if(!m_bInitialized) return;

        //finish builing threads
        for(unsigned n = 0;n < m_vecBuildingThread.size();n++)
        {
            BuilderThread* pThread = m_vecBuildingThread[n];
            pThread->finishMission();
            delete pThread;
            pThread = NULL;
        }

        m_pConvert = NULL;

        m_bInitialized = false;
    }

    bool WFSBuilder::createLogicalLayer()
    {
        logical::ILayer* pRootLayer = m_pLayerManager->getCultureRootLayer();
        if(pRootLayer == NULL)
        {
            return false;
        }

        m_mapFeatureLayer = m_pDriver->getFeatureLayer();
        FEATURELAYERMAP::const_iterator pItor = m_mapFeatureLayer.cbegin();
        while (pItor != m_mapFeatureLayer.cend())
        {
            logical::ILayer* pLayer = pRootLayer->createSubLayer();
            pLayer->setName(pItor->first);
            m_mapLogicalLayer[pItor->first] = pLayer;
            m_listFeature.push_back(pItor->first);
            pItor++;
        }

        return true;
    }

    bool WFSBuilder::getLayerID(const std::string& strLayerName,std::string& strID)
    {
        LOGICALLAYERMAP::iterator pItr = m_mapLogicalLayer.find(strLayerName);
        if(pItr == m_mapLogicalLayer.end())
        {
            return false;
        }
        strID = pItr->second->getID().toString();
        return true;
    }

    bool WFSBuilder::checkFeature()
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxFeature);
        return m_listFeature.empty();
    }

    void WFSBuilder::removeFeature(const std::string strFeature)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxFeature);
        m_listFeature.remove(strFeature);
    }

    bool WFSBuilder::downloadFeature(FEATURELAYERMAP& fMap)
    {
        //获取Feature
        FEATURELAYERMAP::const_iterator pItor = fMap.cbegin();
        while (pItor != fMap.cend())
        {
            deues::IFeatureLayer* pFeatureLayer = pItor->second;
            std::string strGML = pFeatureLayer->getAllFeature();
            if(strGML.empty())
            {
                pItor++;
                continue;
            }
            logical::ILayer* pLogicalLayer = m_mapLogicalLayer[pItor->first];
            if(m_pConvert->gml2Parameter(pLogicalLayer,strGML,pFeatureLayer->getIDProperty(),pFeatureLayer->getGeometryProperty()))
            {
                //removeFeature(pItor->first);
                pItor = fMap.erase(pItor);
                continue;
            }

            pItor++;
        }
        return true;
    }


    void WFSBuilder::BuilderThread::run()
    {
        while((unsigned)m_MissionFinished == 0u)
        {
            // 获取BBox信息
            if(m_featureMap.empty())
            {
                suspend(true);
                m_block.block();
                continue;
            }
            // 下载数据
            m_pWFSBuilder->downloadFeature(m_featureMap);
        }
    }

}
