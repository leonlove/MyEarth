#ifndef STATE_REFRESHING_THREAD_H_1893E690_45A9_4D22_8C24_3F0A4962610C_INCLUDE
#define STATE_REFRESHING_THREAD_H_1893E690_45A9_4D22_8C24_3F0A4962610C_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/op.h>
#include <OpenSP/sp.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <OpenThreads/Block>
#include <IDProvider/ID.h>
#include <list>
#include <Common/IStateImplementer.h>

namespace logical
{

class LayerManager;
class StateRefreshingThread : public OpenThreads::Thread, public OpenSP::Ref
{
public:
    explicit StateRefreshingThread(LayerManager *pLayerManager, cmm::IStateImplementer *pStateImpl);
    virtual ~StateRefreshingThread(void);

    class Task : public OpenSP::Ref
    {
        friend class StateRefreshingThread;
    public:
        explicit Task(const ID &idTarget, const std::string &strStateName, bool bStateEnable)
            : m_idTarget(idTarget),
              m_strStateName(strStateName),
              m_bStateEnable(bStateEnable)
        {
        }

        virtual ~Task(void) {   }

    protected:
        ID              m_idTarget;
        std::string     m_strStateName;
        bool            m_bStateEnable;
    };

protected:
    virtual void run(void);

public:
    void    pushTask(Task *pTask);
    void    finishMission(void);
    void    suspend(bool bSuspend);

protected:
    Task *takeTask(void);

protected:
    OpenSP::op<cmm::IStateImplementer>      m_pStateImplementer;
    LayerManager                   *m_pLayerManager;

    OpenThreads::Block              m_block;
    OpenThreads::Atomic             m_MissionFinished;

    int                             m_nSuspendCount;
    OpenThreads::Mutex              m_mtxSuspendCount;

    OpenThreads::Mutex              m_mtxTaskQueue;
    std::list<OpenSP::sp<Task> >    m_listTaskQueue;
};

}

#endif

