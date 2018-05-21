#ifndef _TILINGTHREADPOOL_H
#define _TILINGTHREADPOOL_H

#include "OpenThreads/Block"
#include "OpenSP/Ref.h"
#include "TilingThread.h"
#include "TaskQueue.h"
#include <list>
#include <DEUDBProxy/IDEUDBProxy.h>

class WaitingThread : public OpenThreads::Thread
{
public:
    WaitingThread(TaskQueue* pTaskQueue, std::list<TilingThread*>& threads)
    {
        m_pTaskQueue = pTaskQueue;
        m_listTilingThreads = threads;
    };
    ~WaitingThread(){};

    void run();

private:
    TaskQueue*               m_pTaskQueue;
    std::list<TilingThread*> m_listTilingThreads;
};

class TilingThreadPool : public OpenSP::Ref
{
public:
    TilingThreadPool(void);
    ~TilingThreadPool(void);

public:
    bool initialize(unsigned int nThreadCount, TaskQueue* pTaskQueue, deudbProxy::IDEUDBProxy *pTargetDB);
    void startMission(void);
    void finishMission(bool bCancel);
    void sleep();
    void wakeup();

private:
    WaitingThread*           m_pWaitingThread;
    TaskQueue*               m_pTaskQueue;
    std::list<TilingThread*> m_listTilingThreads;
};

#endif
