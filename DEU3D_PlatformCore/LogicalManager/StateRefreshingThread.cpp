#include "StateRefreshingThread.h"
#include "LayerManager.h"

namespace logical
{

StateRefreshingThread::StateRefreshingThread(LayerManager *pLayerManager, cmm::IStateImplementer *pStateImpl)
    : m_pLayerManager(pLayerManager),
      m_pStateImplementer(pStateImpl),
      m_nSuspendCount(0u)
{
    setStackSize(64u * 1024u);
    m_MissionFinished.exchange(0u);
    m_block.release();
}


StateRefreshingThread::~StateRefreshingThread(void)
{
}


void StateRefreshingThread::pushTask(Task *pTask)
{
    if(!pTask)  return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTaskQueue);
    m_listTaskQueue.push_back(pTask);
    suspend(false);
}


StateRefreshingThread::Task *StateRefreshingThread::takeTask(void)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTaskQueue);
    if(m_listTaskQueue.empty())     return NULL;

    OpenSP::sp<Task> pTask = m_listTaskQueue.front();
    m_listTaskQueue.pop_front();
    return pTask.release();
}


void StateRefreshingThread::finishMission(void)
{
    m_MissionFinished.exchange(1);
    m_block.release();
    join();
}

void StateRefreshingThread::suspend(bool bSuspend)
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


void StateRefreshingThread::run(void)
{
    while((unsigned)m_MissionFinished == 0u)
    {
        m_block.block();

        OpenSP::sp<Task> pTask = takeTask();
        if(!pTask.valid())
        {
            suspend(true);
            continue;
        }

        IDList listObjects;

        const ID &idTarget = pTask->m_idTarget;
        if(idTarget.ObjectID.m_nType == CULTURE_LAYER_ID)
        {
            OpenSP::sp<Layer> pLayer = m_pLayerManager->findLayerInCache(idTarget);
            if(!pLayer.valid())
            {
                continue;
            }

            const bool bRet = pLayer->getAllInstances(listObjects, pTask->m_strStateName, pTask->m_bStateEnable);
            if(!bRet)
            {
                pushTask(pTask.get());
                continue;
            }
        }
        else if(idTarget.ObjectID.m_nType == PARAM_POINT_ID
             || idTarget.ObjectID.m_nType == PARAM_LINE_ID
             || idTarget.ObjectID.m_nType == PARAM_FACE_ID)
        {
            listObjects.push_back(idTarget);
        }
        else
        {
            continue;
        }

        if(!listObjects.empty())
        {
            OpenSP::sp<cmm::IStateImplementer>  pStateImpl;
            if(!m_pStateImplementer.lock(pStateImpl))
            {
                break;
            }

            pStateImpl->refreshObjectState(listObjects, pTask->m_strStateName, pTask->m_bStateEnable);
        }
    }
}


}