#include <osg/SharedObjectPool>

using namespace osg;

SharedObjectPool *SharedObjectPool::instance(void)
{
    static ref_ptr<SharedObjectPool>   s_pSharedObjectPool = new SharedObjectPool;
    return s_pSharedObjectPool.get();
}


UINT_64 getRefTime(void)
{
    static OpenThreads::Mutex mtxUpdateTime;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mtxUpdateTime);

    static UINT_64 nUpdateTime = 0u;
    nUpdateTime++;
    return nUpdateTime;
}


void SharedObjectPool::TrimmerThread::run(void)
{
    while(0u == (unsigned)m_MissionFinished)
    {
        m_pThis->trimePool();
        OpenThreads::Thread::microSleep(400u * 1000u);
    }
}


bool SharedObjectPool::initialize(void)
{
    if(m_pTrimmerThread.valid())    return true;

    m_pTrimmerThread = new TrimmerThread(this);
    m_pTrimmerThread->startThread();

    return true;
}


void SharedObjectPool::trimePool(void)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    std::map<ObjMark, SharedObject>::iterator itor = m_mapObjectPool.begin();
    while(itor != m_mapObjectPool.end())
    {
        SharedObject &obj = itor->second;
        if(!obj.m_pObject.valid())
        {
            m_mapObjectRefTime.erase(obj.m_nLastRefTime);
            itor = m_mapObjectPool.erase(itor);
            continue;
        }

        const ObjMark &mark = itor->first;
        if(!mark.m_id.isValid() && !mark.m_pImage.valid())
        {
            m_mapObjectRefTime.erase(obj.m_nLastRefTime);
            itor = m_mapObjectPool.erase(itor);
            continue;
        }

        ++itor;
    }
}


void SharedObjectPool::addObject(const ID &id, Object *pObject)
{
    if(!id.isValid())   return;
    if(!pObject)        return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    const UINT_64 nRefTime = getRefTime();

    const ObjMark mark(id);
    m_mapObjectRefTime[nRefTime] = mark;

    std::map<ObjMark, SharedObject>::iterator itorFind = m_mapObjectPool.find(mark);
    if(itorFind != m_mapObjectPool.end())
    {
        SharedObject &obj = itorFind->second;
        obj.m_pObject = pObject;
        m_mapObjectRefTime.erase(obj.m_nLastRefTime);

        obj.m_nLastRefTime = nRefTime;
        return;
    }

    SharedObject  &obj = m_mapObjectPool[mark];
    obj.m_nLastRefTime = nRefTime;
    obj.m_pObject      = pObject;
}


void SharedObjectPool::addTexture(Image *pImage, Texture *pTexture)
{
    if(!pImage)     return;
    if(!pTexture)   return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    const UINT_64 nRefTime = getRefTime();

    const ObjMark mark(pImage);
    m_mapObjectRefTime[nRefTime] = mark;

    std::map<ObjMark, SharedObject>::iterator itorFind = m_mapObjectPool.find(mark);
    if(itorFind != m_mapObjectPool.end())
    {
        SharedObject &obj = itorFind->second;
        obj.m_pObject = pTexture;
        m_mapObjectRefTime.erase(obj.m_nLastRefTime);

        obj.m_nLastRefTime = nRefTime;
        return;
    }

    SharedObject  &obj = m_mapObjectPool[mark];
    obj.m_nLastRefTime = nRefTime;
    obj.m_pObject      = pTexture;
}


bool SharedObjectPool::findObject(const ID &id, ref_ptr<Object> &pObject)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    const ObjMark mark(id);
    std::map<ObjMark, SharedObject>::iterator itorFind = m_mapObjectPool.find(mark);
    if(itorFind == m_mapObjectPool.end())
    {
        return false;
    }

    const UINT_64 nRefTime = getRefTime();
    SharedObject &obj = itorFind->second;

    m_mapObjectRefTime.erase(obj.m_nLastRefTime);

    obj.m_nLastRefTime = nRefTime;
    m_mapObjectRefTime[nRefTime] = mark;

    return obj.m_pObject.lock(pObject);
}


bool SharedObjectPool::findObject(const osg::Image *pImage, ref_ptr<Texture> &pTexture)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    const ObjMark mark(const_cast<osg::Image *>(pImage));
    std::map<ObjMark, SharedObject>::iterator itorFind = m_mapObjectPool.find(mark);
    if(itorFind == m_mapObjectPool.end())
    {
        return false;
    }

    const UINT_64 nRefTime = getRefTime();
    SharedObject &obj = itorFind->second;

    m_mapObjectRefTime.erase(obj.m_nLastRefTime);

    obj.m_nLastRefTime = nRefTime;
    m_mapObjectRefTime[nRefTime] = mark;

    osg::ref_ptr<osg::Object>   pObject;
    if(!obj.m_pObject.lock(pObject))
    {
        return false;
    }

    pTexture = dynamic_cast<Texture *>(pObject.get());
    return pTexture.valid();
}


void SharedObjectPool::clearObjectByDataset(unsigned nDataset)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectPool);

    if(nDataset == ~0u)
    {
        m_mapObjectPool.clear();
        return;
    }
    std::map<ObjMark, SharedObject>::iterator itor = m_mapObjectPool.begin();
    while(itor != m_mapObjectPool.end())
    {
        if(itor->first.m_id.TileID.m_nDataSetCode == nDataset)
        {
            itor = m_mapObjectPool.erase(itor);
            if(itor == m_mapObjectPool.end())
            {
                break;
            }
            continue;
        }
        else if(itor->first.m_pImage.valid() && itor->first.m_pImage->getID().TileID.m_nDataSetCode == nDataset)
        {
            itor = m_mapObjectPool.erase(itor);
            if(itor == m_mapObjectPool.end())
            {
                break;
            }
            continue;
        }
        else
        {
            ++itor;
        }
    }

    return;
}

