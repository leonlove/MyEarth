#ifndef _WFS_BUILDER_H_68F9753C_D250_4F22_A802_2A5E51B1036B_
#define _WFS_BUILDER_H_68F9753C_D250_4F22_A802_2A5E51B1036B_

#include <list>
#include "IWFSBuilder.h"
#include <ExternalService/IFeatureLayer.h>
#include <LogicalManager/ILayer.h>
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventReceiver.h>
#include "..\ModelBuilder\IModelBuilder.h"
#include <common/Pyramid.h>
#include "ConvertGML.h"



namespace wfsb
{

    class WFSBuilder : public IWFSBuilder
    {
    public:
        explicit WFSBuilder(void);
        virtual ~WFSBuilder(void);
        bool initialize(deues::IWFSDriver*    pDriver,
                        logical::ILayerManager* pLayerManager);
        void unInitialize();
        bool getLayerID(const std::string& strLayerName,std::string& strID);
        bool downloadFeature(FEATURELAYERMAP& fMap);

    protected:
        bool createLogicalLayer();
        bool checkFeature();
        void removeFeature(const std::string strFeature);

    protected:
        bool                               m_bInitialized;
        FEATURELAYERMAP                    m_mapFeatureLayer;
        LOGICALLAYERMAP                    m_mapLogicalLayer;
        OpenSP::sp<deues::IWFSDriver>      m_pDriver;
        OpenSP::sp<logical::ILayerManager> m_pLayerManager;
        OpenSP::sp<ConvertGML>             m_pConvert;
        std::list<std::string>             m_listFeature;
        OpenSP::sp<IModelBuilder>          m_pModelBuilder;
        OpenThreads::Mutex                 m_mtxFeature;
        ////////////////////////////////////////////////////////////////////////////////////////////////
    protected:
        class BuilderThread : public OpenThreads::Thread, public OpenSP::Ref
        {
        public:
            explicit BuilderThread(WFSBuilder *pWFSBuilder,const FEATURELAYERMAP& fMap)
            {
                setStackSize(128u * 1024u);
                m_pWFSBuilder = pWFSBuilder;
                m_featureMap = fMap;
                m_MissionFinished.exchange(0u);
                m_nSuspendCount = 0;
                m_block.release();
            }
        public:
            void suspend(bool bSuspend)
            {
                if((unsigned)m_MissionFinished == 1u)
                {
                    return;
                }

                OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxSuspendCount);
                if(bSuspend)
                {
                    ++m_nSuspendCount;
                }
                else
                {
                    --m_nSuspendCount;
                }

                m_block.set(m_nSuspendCount <= 0);
            }

            void finishMission(void)
            {
                m_MissionFinished.exchange(1u);
                m_block.set(true);
                join();
            }

        protected:
            virtual void run(void);

        protected:
            WFSBuilder*             m_pWFSBuilder;
            FEATURELAYERMAP         m_featureMap;
            OpenThreads::Block      m_block;
            OpenThreads::Atomic     m_MissionFinished;
            int                     m_nSuspendCount;
            OpenThreads::Mutex      m_mtxSuspendCount;
        };

    protected:
        std::vector<BuilderThread*>    m_vecBuildingThread;
        volatile unsigned              m_nThread;

        

    };

}

#endif  //_WFS_BUILDER_H_68F9753C_D250_4F22_A802_2A5E51B1036B_

