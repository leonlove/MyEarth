#ifndef _BBOX_QUEUE_H_148DF6CF_651C_46DB_A7A1_6846DFC8E73A_
#define _BBOX_QUEUE_H_148DF6CF_651C_46DB_A7A1_6846DFC8E73A_

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
#include "WFSDefine.h"

namespace wfsb
{

class IWFSBuilder;

class BBoxQueue : public OpenSP::Ref
{
public:
    BBoxQueue(void);
    ~BBoxQueue(void);
    bool        initialize(ea::IEventAdapter* pIEventAdapter,
                           IWFSBuilder *pWFSBuilder);
    void        unInitialize();
    bool        getBBoxInfo(BBoxInfo& bInfo);
    bool        addBBoxInfo(BBoxInfo  bInfo);

protected:
    std::list<BBoxInfo>                     m_listBBox;
    OpenSP::sp<ea::IEventAdapter>           m_pEventAdapter;
    OpenThreads::Mutex                      m_mtxBBoxQueue;
    OpenSP::sp<IWFSBuilder>                 m_pWFSBuilder;
    bool                                    m_bInitialized;

protected:
    class BBoxReceiver : public ea::IEventReceiver
    {
    public:
        BBoxReceiver(BBoxQueue* pBBoxQueue) {m_pBBoxQueue = pBBoxQueue;}
        ~BBoxReceiver(){}

    protected:
        virtual void onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject);
    protected:
        OpenSP::sp<BBoxQueue> m_pBBoxQueue;
    };
protected:
    OpenSP::sp<BBoxReceiver> m_pBBoxReceiver;

protected:
    class BuilderThread : public OpenThreads::Thread, public OpenSP::Ref
    {
    public:
        explicit BuilderThread(IWFSBuilder *pWFSBuilder,BBoxQueue* pBBoxQueue)
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
        BBoxQueue*              m_pBBoxQueue;
        OpenThreads::Block      m_block;
        OpenThreads::Atomic     m_MissionFinished;
        int                     m_nSuspendCount;
        OpenThreads::Mutex      m_mtxSuspendCount;
    };

    OpenSP::sp<BuilderThread>   m_pBuilderThread;
};

}
#endif //_BBOX_QUEUE_H_148DF6CF_651C_46DB_A7A1_6846DFC8E73A_

