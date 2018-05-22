#include "SharedObjectPool.h"

#include <osg/Object>

SharedObjectPool::SharedObjectPool(void)
{
    m_pThread = new SharedObjectThread(this);
    m_pThread->startThread();
}


SharedObjectPool::~SharedObjectPool(void)
{
    m_pThread = NULL;
}

void SharedObjectPool::trimePool()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);
    std::map<const ID, osg::observer_ptr<osg::Object> >::iterator itor = m_mapObject.begin();

    //printf("SharedObjectPool Size is %d\n", m_mapObject.size());

    while(itor != m_mapObject.end())
    {
        if(itor->second.valid())
        {
            ++itor;
        }
        else
        {
            itor = m_mapObject.erase(itor);
        }
    }
}

void SharedObjectPool::addObject(const ID &id, osg::Object *pObject)
{
    if(!id.isValid()) return;
    if(!pObject) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

     osg::observer_ptr<osg::Object> pTempObject = pObject;

     m_mapObject[id] = pTempObject;

}

bool SharedObjectPool::findObject(const ID &id, osg::ref_ptr<osg::Object> &pObject)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    std::map<const ID, osg::observer_ptr<osg::Object> >::iterator itor = m_mapObject.find(id);
    if(itor == m_mapObject.end())
    {
        return false;
    }

    if(itor->second.valid())
    {
        itor->second.lock(pObject);
        return true;
    }

    return false;
}

SharedObjectPool::SharedObjectThread::SharedObjectThread(SharedObjectPool *pPool)
{
    m_pPool = pPool;
}

SharedObjectPool::SharedObjectThread::~SharedObjectThread(void)
{
    cancel();
}

int SharedObjectPool::SharedObjectThread::cancel()
{
    int result = 0;

    if( isRunning() )
    {
        setDone(true);

        join();
    }

    return result;
}

void SharedObjectPool::SharedObjectThread::run()
{
    do{
        if (m_done)
        {
            break;
        }
        m_pPool->trimePool();
        OpenThreads::Thread::microSleep(1000000u);
        OpenThreads::Thread::YieldCurrentThread();
    } while (!testCancel() && !m_done);
}