#include "InitializationThread.h"
#include <algorithm>
#include "Layer.h"

namespace logical
{

InitializationThread::InitializationThread(InitializationThreadPool *pPool)
    : m_pThreadPool(pPool),
      m_nSuspendCount(0)
{
    setStackSize(128u * 1024u);
    m_MissionFinished.exchange(0u);
    m_block.release();
}


InitializationThread::~InitializationThread(void)
{
}


void InitializationThread::run(void)
{
    while((unsigned)m_MissionFinished == 0u)
    {
        m_block.block();

        OpenSP::sp<Task> pTask = m_pThreadPool->takeFront();
        if(!pTask.valid())
        {
            suspend(true);
            continue;
        }

        pTask->m_pLayer->init();
        pTask->m_blockSignal.set(true);
        m_pThreadPool->onTaskFinished(pTask->m_pLayer);
        //OpenThreads::Thread::microSleep(500u * 1000u);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
InitializationThreadPool::InitializationThreadPool(void)
{
}


InitializationThreadPool::~InitializationThreadPool(void)
{
    std::vector<InitializationThread *>::iterator itorThread = m_vecThreads.begin();
    for( ; itorThread != m_vecThreads.end(); ++itorThread)
    {
        InitializationThread *pThread = *itorThread;
        pThread->finishMission();
        delete pThread;
    }
}


bool InitializationThreadPool::initialize(unsigned nThreadCount)
{
    if(nThreadCount < 1u)   return false;

    assert(m_vecThreads.empty());

    for(unsigned n = 0u; n < nThreadCount; n++)
    {
        InitializationThread *pThread = new InitializationThread(this);
        pThread->startThread();
        m_vecThreads.push_back(pThread);
    }

    return true;
}


void InitializationThreadPool::addTask(Layer *pLayer)
{
    if(!pLayer) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTask);

    std::list<OpenSP::sp<InitializationThread::Task> >::iterator itorFind
        = std::find_if(m_listTasks.begin(), m_listTasks.end(), TaskFinder(pLayer));

    if(itorFind == m_listTasks.end())
    {
        InitializationThread::Task *pNewTask = new InitializationThread::Task;
        pNewTask->m_pLayer = pLayer;
        pNewTask->m_blockSignal.set(false);
        m_listTasks.push_back(pNewTask);
    }

    for(unsigned n = 0u; n < m_vecThreads.size(); n++)
    {
        InitializationThread *pThread = m_vecThreads[n];
        pThread->suspend(false);
    }
}


void InitializationThreadPool::raisePriority(const Layer *pLayer)
{
    OpenSP::sp<InitializationThread::Task> pTask = NULL;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTask);

        std::list<OpenSP::sp<InitializationThread::Task> >::iterator itorFind
            = std::find_if(m_listTasks.begin(), m_listTasks.end(), TaskFinder(pLayer));

        if(itorFind != m_listTasks.end())
        {
            pTask = *itorFind;

            if(itorFind != m_listTasks.begin())
            {
                m_listTasks.erase(itorFind);
                m_listTasks.push_front(pTask);
            }
        }
    }

    if(!pTask.valid())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDoingTask);
        std::map<const Layer *, OpenSP::sp<InitializationThread::Task> >::iterator itorFind = m_mapDoingTasks.find(pLayer);
        if(itorFind == m_mapDoingTasks.end())
        {
            return;
        }
        pTask = itorFind->second;
    }

    if(pTask.valid())
    {
        pTask->m_blockSignal.block();
    }
}


InitializationThread::Task *InitializationThreadPool::takeFront(void)
{
    OpenSP::sp<InitializationThread::Task> pTask = NULL;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTask);
        if(m_listTasks.empty())
        {
            return NULL;
        }

        pTask = m_listTasks.front();
        m_listTasks.pop_front();
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDoingTask);
        m_mapDoingTasks[pTask->m_pLayer.get()] = pTask;
    }

    return pTask.release();
}


void InitializationThreadPool::onTaskFinished(Layer *pLayer)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDoingTask);
    m_mapDoingTasks.erase(pLayer);
}


bool InitializationThreadPool::isBusy(void) const
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDoingTask);
        if(!m_listTasks.empty())
        {
            return true;
        }
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDoingTask);
        if(!m_mapDoingTasks.empty())
        {
            return true;
        }
    }

    return false;
}


}

