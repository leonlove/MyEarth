#ifndef _AREA_INFO_QUEUE_H_77DF5C5C_97D9_4BBB_BD17_AE081FADF318_
#define _AREA_INFO_QUEUE_H_77DF5C5C_97D9_4BBB_BD17_AE081FADF318_

#include <string>
#include <list>
#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>
#include <OpenThreads/Mutex>
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventReceiver.h>
#include "export.h"

namespace wfsb
{

    class IWFSBuilder;

    class AreaInfoQueue : public OpenSP::Ref
    {
    public:
        AreaInfoQueue(void);
        ~AreaInfoQueue(void);
        bool        initialize(ea::IEventAdapter* pIEventAdapter,IWFSBuilder *pWFSBuilder);
        void        unInitialize();
        bool        getBBoxInfo(AreaInfo& bInfo);
        bool        addBBoxInfo(AreaInfo  bInfo);
    protected:
        std::list<AreaInfo>                     m_listBBox;
        OpenSP::sp<ea::IEventAdapter>           m_pEventAdapter;
        OpenThreads::Mutex                      m_mtxBBoxQueue;
        OpenSP::sp<IWFSBuilder>                 m_pWFSBuilder;
        bool                                    m_bInitialized;

    protected:
        class AreaReceiver : public ea::IEventReceiver
        {
        public:
            AreaReceiver(AreaInfoQueue* pBBoxQueue) {m_pBBoxQueue = pBBoxQueue;}
            ~AreaReceiver(){}

        protected:
            virtual void onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject);
        protected:
            OpenSP::sp<AreaInfoQueue> m_pBBoxQueue;
        };
    protected:
        OpenSP::sp<AreaReceiver> m_pBBoxReceiver;

    protected:
        class BuilderThread : public OpenThreads::Thread, public OpenSP::Ref
        {
        public:
            explicit BuilderThread(IWFSBuilder *pWFSBuilder,AreaInfoQueue* pBBoxQueue)
            {
                setStackSize(128u * 1024u);
                m_pWFSBuilder = pWFSBuilder;
                m_pBBoxQueue  = pBBoxQueue;
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
            IWFSBuilder*            m_pWFSBuilder;
            AreaInfoQueue*          m_pBBoxQueue;
            OpenThreads::Block      m_block;
            OpenThreads::Atomic     m_MissionFinished;
            int                     m_nSuspendCount;
            OpenThreads::Mutex      m_mtxSuspendCount;
        };

        OpenSP::sp<BuilderThread>   m_pBuilderThread;
    };

}
#endif //_AREA_INFO_QUEUE_H_77DF5C5C_97D9_4BBB_BD17_AE081FADF318_

