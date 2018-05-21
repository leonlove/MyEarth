#include "TaskQueue.h"
#include "TilingThreadPool.h"
#include <Common\Common.h>

TaskQueue::TaskQueue(void)
: m_nMaxTaskLen(1024)
, m_bHeightField(false)
, m_nTileSize(256)
, m_nTopLevel(1)
, m_nDataSetCode(0)
, m_pIEventAdapter(NULL)
{
    m_blockLength.set(true);
}


TaskQueue::~TaskQueue(void)
{
    m_listTasks.clear();
}

void TaskQueue::setHeightField(const bool bHeightField)
{
    m_bHeightField = bHeightField;

    return;
}

bool TaskQueue::getHeightField()
{
    return m_bHeightField;
}

void TaskQueue::setTileSize(const unsigned int nSize)
{
    m_nTileSize = nSize;

    return;
}

void TaskQueue::setIEventAdapter(ea::IEventAdapter* pIEventAdapter)
{
    m_pIEventAdapter = pIEventAdapter;
}

ea::IEventAdapter* TaskQueue::getIEventAdapter()
{
    return m_pIEventAdapter;
}

void TaskQueue::setInvalidColor(std::vector<INVALIDCOLOR> vecInvalidColor)
{
    m_vecInvalidColor = vecInvalidColor;
}

std::vector<INVALIDCOLOR> TaskQueue::getInvalidColor()
{
    return m_vecInvalidColor;
}

unsigned int TaskQueue::getTileSize()
{
    return m_nTileSize;
}

void TaskQueue::setDataSetCode(const unsigned int nDataSetCode)
{
    m_nDataSetCode = nDataSetCode;
}

unsigned int TaskQueue::getDataSetCode()
{
    return m_nDataSetCode;
}

void TaskQueue::setGlobeUniqueNumber()
{
    m_nUniqueID = cmm::genGlobeUniqueNumber();
}

unsigned __int64 TaskQueue::getGlobeUniqueNumber()
{
    return m_nUniqueID;
}

void TaskQueue::addTopLevelID(const ID &id)
{
    if (id.TileID.m_nLevel == m_nTopLevel)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxTopLevel);

        m_IDVec.push_back(id);
    }

    return;
}

void TaskQueue::getTopLevelID(std::vector<ID> &IDVec)
{
    IDVec = m_IDVec;

    return;
}

void TaskQueue::clearTopLevelID()
{
    m_IDVec.clear();
}

bool TaskQueue::addTask(const OpenSP::sp<TilingTask> &pTask)
{
    m_blockLength.block();

    unsigned int nCurLen = 0;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxTaskQueue);
        m_listTasks.push_back(pTask);
        nCurLen = m_listTasks.size();
    }

    if(nCurLen > 1024)
    {
        m_blockLength.set(false);
    }

    return true;
}

bool TaskQueue::takeTask(OpenSP::sp<TilingTask> &task)
{
    unsigned int nCurLen = 0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxTaskQueue);
        if (m_listTasks.empty())
        {
            return false;
        }

        task = m_listTasks.front();
        m_listTasks.pop_front();
        nCurLen = m_listTasks.size();
    }

    if(nCurLen <= 1024)
    {
        m_blockLength.set(true);
    }

    return true;
}

void TaskQueue::Stop()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxTaskQueue);

    m_listTasks.clear();

    m_blockLength.set(true);

    m_IDVec.clear();
}

bool TaskQueue::isEmpty()
{
    return m_listTasks.empty();
}

void TaskQueue::setTopLevel(const unsigned int nTopLevel)
{
    m_nTopLevel = nTopLevel;

    return;
}
