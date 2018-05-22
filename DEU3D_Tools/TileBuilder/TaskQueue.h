#ifndef _TASKQUEUE_H
#define _TASKQUEUE_H

#include "ITileBuilder.h"
#include "Source.h"
#include "OpenSP/Ref.h"
#include "OpenSP/sp.h"
#include "OpenThreads/Mutex"
#include "OpenThreads/Block"
#include "DEUDB/IDEUDB.h"
#include "EventAdapter/IEventAdapter.h"
#include <vector>
#include <list>

struct TilingTask : public OpenSP::Ref
{
    unsigned m_nLevel;
    unsigned m_nRow;
    unsigned m_nCol;
    std::vector<OpenSP::sp<Source>> m_vecSourceFiles;
};

class TaskQueue : public OpenSP::Ref
{
public:
    TaskQueue(void);
    ~TaskQueue(void);

public:
    void               Stop();
    bool               addTask(const OpenSP::sp<TilingTask>& pTask);
    bool               takeTask(OpenSP::sp<TilingTask> &task);
    bool               isEmpty();
    void               setTopLevel(const unsigned int nTopLevel);
    void               setHeightField(const bool bHeightField);
    bool               getHeightField();
    void               addTopLevelID(const ID &id);
    void               getTopLevelID(std::vector<ID> &IDVec);
    void               clearTopLevelID();
    void               setTileSize(const unsigned int nSize);
    unsigned int       getTileSize();
    void               setDataSetCode(const unsigned int m_nDataSetCode);
    unsigned int       getDataSetCode();
    void               setGlobeUniqueNumber();
    unsigned __int64   getGlobeUniqueNumber();
    void               setIEventAdapter(ea::IEventAdapter* pIEventAdapter);
    ea::IEventAdapter* getIEventAdapter();
    void               setInvalidColor(std::vector<INVALIDCOLOR> vecInvalidColor);
    std::vector<INVALIDCOLOR> getInvalidColor();

private:
    bool               m_bHeightField;
    unsigned int       m_nTileSize;
    unsigned int       m_nMaxTaskLen;
    unsigned int       m_nTopLevel;
    unsigned int       m_nDataSetCode;
    unsigned __int64   m_nUniqueID;
    OpenThreads::Mutex m_mtxTopLevel;
    OpenThreads::Mutex m_mtxTaskQueue;
    OpenThreads::Mutex m_mtxSaveFile;
    OpenThreads::Block m_blockLength;
    std::vector<ID>    m_IDVec;
    std::list<OpenSP::sp<TilingTask>> m_listTasks;
    ea::IEventAdapter* m_pIEventAdapter;
    std::vector<INVALIDCOLOR> m_vecInvalidColor;
};

#endif
