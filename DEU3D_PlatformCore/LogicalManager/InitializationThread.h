#ifndef INITIALIZATION_H_90D131C0_0B1B_4FD8_B2CF_E78CCB7399F4_INCLUDE
#define INITIALIZATION_H_90D131C0_0B1B_4FD8_B2CF_E78CCB7399F4_INCLUDE

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>
#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <vector>
#include <list>
#include <map>
#include "Object.h"

namespace logical
{
class InitializationThreadPool;

class InitializationThread : public OpenThreads::Thread
{
public:
    explicit InitializationThread(InitializationThreadPool *pPool);
    virtual ~InitializationThread(void);

public:
    void finishMission(void)
    {
        m_MissionFinished.exchange(1);
        m_block.set(true);
        join();
    }

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

    struct Task : public OpenSP::Ref
    {
        OpenSP::sp<Layer>       m_pLayer;
        OpenThreads::Block      m_blockSignal;
    };

protected:
    virtual void run(void);

protected:
    InitializationThreadPool       *m_pThreadPool;

    OpenThreads::Block      m_block;
    OpenThreads::Atomic     m_MissionFinished;

    int                     m_nSuspendCount;
    OpenThreads::Mutex      m_mtxSuspendCount;
};


class InitializationThreadPool : public OpenSP::Ref
{
public:
    explicit InitializationThreadPool(void);
    virtual ~InitializationThreadPool(void);

public:
    bool initialize(unsigned nThreadCount);
    void addTask(Layer *pLayer);
    void raisePriority(const Layer *pLayer);
    bool isBusy(void) const;

protected:
    struct TaskFinder
    {
        const Layer *m_pFindTarget;
        TaskFinder(const Layer *pTarget) : m_pFindTarget(pTarget){}
        bool operator()(const OpenSP::sp<InitializationThread::Task> &itor)
        {
            return itor->m_pLayer == m_pFindTarget;
        }
    };

    friend class InitializationThread;
    InitializationThread::Task *takeFront(void);
    void onTaskFinished(Layer *pLayer);

protected:
    std::vector<InitializationThread *>                 m_vecThreads;

    mutable OpenThreads::Mutex                          m_mtxTask;
    std::list<OpenSP::sp<InitializationThread::Task> >  m_listTasks;

    mutable OpenThreads::Mutex                          m_mtxDoingTask;
    std::map<const Layer *, OpenSP::sp<InitializationThread::Task> >    m_mapDoingTasks;
};

}
#endif
