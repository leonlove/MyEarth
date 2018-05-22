#include "TilingThreadPool.h"
#include "EventAdapter/IEventObject.h"

void WaitingThread::run()
{
    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        (*it)->join();
    }

    unsigned int nFinish = 1;
    unsigned int nState  = 3;
    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
    if (pEventObject == NULL)
    {
        return;
    }

    pEventObject->setAction(ea::ACTION_TILE_BUILDER);
    pEventObject->putExtra("STATE", nState);
    pEventObject->putExtra("Finish", nFinish);
    m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());
}

TilingThreadPool::TilingThreadPool(void)
: m_listTilingThreads(NULL)
, m_pWaitingThread(NULL)
{
}


TilingThreadPool::~TilingThreadPool(void)
{
    if (m_pWaitingThread != NULL)
    {
        m_pWaitingThread->join();
        delete m_pWaitingThread;
        m_pWaitingThread = NULL;
    }

    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        delete *it;
    }
    m_listTilingThreads.clear();
}


bool TilingThreadPool::initialize(unsigned int nThreadCount, TaskQueue* pTaskQueue, deudbProxy::IDEUDBProxy *pTargetDB)
{
    if (pTargetDB == NULL || pTargetDB == NULL)
    {
        return false;
    }

    //清理以前残留数据
    if (m_pWaitingThread != NULL)
    {
        m_pWaitingThread->join();
        delete m_pWaitingThread;
        m_pWaitingThread = NULL;
    }

    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        delete *it;
    }
    m_listTilingThreads.clear();

    //初始化新数据
    m_pTaskQueue = pTaskQueue;

    m_listTilingThreads.clear();

    unsigned int nNum = nThreadCount <=0 ? 4 : nThreadCount;

    for (unsigned int i=0; i<nNum; i++)
    {
        TilingThread *pThread = new TilingThread(pTaskQueue, pTargetDB);

        m_listTilingThreads.push_back(pThread);
    }

    return true;
}

void TilingThreadPool::startMission(void)
{
    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        (*it)->startThread();
    }

    return;
}

void TilingThreadPool::finishMission(bool bCancel)
{
    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        (*it)->finishMission(bCancel);
    }

    if (m_pWaitingThread == NULL)
    {
        m_pWaitingThread = new WaitingThread(m_pTaskQueue, m_listTilingThreads);
        m_pWaitingThread->startThread();
    }

    if(!bCancel)
    {
        m_pWaitingThread->join();
    }

    return;
}

void TilingThreadPool::sleep()
{
    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        (*it)->sleep();
    }

    return;
}

void TilingThreadPool::wakeup()
{
    for (std::list<TilingThread*>::const_iterator it=m_listTilingThreads.cbegin(); it!=m_listTilingThreads.cend(); ++it)
    {
        (*it)->wakeup();
    }

    return;
}
