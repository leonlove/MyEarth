#ifndef PARAM_RECTIFY_THREAD_TOOL_H_37EBB884_9AEE_4D02_AA06_741C03596BDB_INCLUDE
#define PARAM_RECTIFY_THREAD_TOOL_H_37EBB884_9AEE_4D02_AA06_741C03596BDB_INCLUDE

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Camera>
#include <osgTerrain/TerrainTile>

#include <OpenSP/Ref.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>

#include <vector>

#include "ParameterNode.h"

class ParmRectifyThreadPool : public OpenSP::Ref
{
public:
    explicit ParmRectifyThreadPool(void);
    virtual ~ParmRectifyThreadPool(void);

public:
    void    setUpThreads(unsigned int nTotalNumThreads);
    int     cancel();
    bool    isRunning() const;
    void    requestParmRectify(ParameterNode *pParameterNode, OpenSP::sp<OpenSP::Ref>& ParmRectifyRequestRef);
    void    setTerrainNode(osg::Node *pTerrainNode) { m_pTerrainNode = pTerrainNode; }
protected:
    unsigned int    addParmRectifyThread();
    void            computeHeightFiled(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos);
    void            computeHeightFiled(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos, double &dblDistance);
    void            computeHeightFiled1(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos);

public:
    class ParmRectifyThread : public osg::Referenced, public OpenThreads::Thread
    {
    public:
        explicit ParmRectifyThread(ParmRectifyThreadPool *pThreadPool)  :
            m_Done(false),
            m_bActive(false),
            m_pThreadPool(pThreadPool)
            {

            }

        void setDone(bool bDone) { m_Done.exchange(bDone ? 1 : 0); }
        bool getDone() const { return m_Done != 0; }

        void setActive(bool bActive) { m_bActive = bActive; }
        bool getActive() const { return m_bActive; }

        virtual int cancel();

        virtual void run();

    protected:
        struct SortFunctor
        {
            SortFunctor(bool bXIncrease, bool bYIncrease)
            {
                m_bXIncrease = bXIncrease;
                m_bYIncrease = bYIncrease;
            }

            bool operator() (osg::Vec3d &lhs, osg::Vec3d &rhs) const
            {
                if(m_bXIncrease && m_bYIncrease)
                {
                    if(rhs._v[0] > lhs._v[0] && rhs._v[1] > lhs._v[1])
                    {
                        return true;
                    }
                }
                else if(m_bXIncrease && !m_bYIncrease)
                {
                    if(rhs._v[0] > lhs._v[0] && rhs._v[1] < lhs._v[1])
                    {
                        return true;
                    }
                }
                else if(!m_bXIncrease && !m_bYIncrease)
                {
                    if(rhs._v[0] < lhs._v[0] && rhs._v[1] < lhs._v[1])
                    {
                        return true;
                    }
                }
                else if(!m_bXIncrease && m_bYIncrease)
                {
                    if(rhs._v[0] < lhs._v[0] && rhs._v[1] > lhs._v[1])
                    {
                        return true;
                    }
                }

                return false;
            }
            bool m_bXIncrease;
            bool m_bYIncrease;
        };

        virtual ~ParmRectifyThread(void)
        {
            cancel();
        }

        OpenThreads::Atomic     m_Done;
        volatile bool           m_bActive;
        ParmRectifyThreadPool   *m_pThreadPool;
    };

    class ParmRectifyTask : public OpenSP::Ref
    {
    public:
        explicit ParmRectifyTask(void) {}
        virtual ~ParmRectifyTask(void) {}

    public:
        osg::observer_ptr<ParameterNode>    m_pParameterNode;
    };

    class ParmRectifyTaskQueue : public OpenSP::Ref
    {
    public:
        explicit ParmRectifyTaskQueue(void) {}
        virtual ~ParmRectifyTaskQueue(void) {}

        void addTask(ParmRectifyTask *pTask)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_TaskMutex);
            addTaskNoLock(pTask);
        }

        void addTaskNoLock(ParmRectifyTask *pTask)
        {
            m_TaskList.push_back(pTask);
        }

        void removeTask(ParmRectifyTask *pTask)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_TaskMutex);
            for(std::list<OpenSP::sp<ParmRectifyTask> >::iterator itor = m_TaskList.begin(); itor != m_TaskList.end(); ++itor)
            {
                if(itor->get() == pTask)
                {
                    m_TaskList.erase(itor);
                    return;
                }
            }
        }

        void takeFirst(OpenSP::sp<ParmRectifyTask> &task)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_TaskMutex);

            if (m_TaskList.empty())
            {
                return;
            }

            task = m_TaskList.front();
            m_TaskList.pop_front();
        }

        std::list<OpenSP::sp<ParmRectifyTask> >     m_TaskList;
        OpenThreads::Mutex                          m_TaskMutex;
    };

protected:
    bool                                            m_bStartThreadCalled;
    std::vector<osg::ref_ptr<ParmRectifyThread> >   m_vecParamThreadList;
    OpenSP::sp<ParmRectifyTaskQueue>                m_TaskQueue;
    osg::ref_ptr<osg::Node>                         m_pTerrainNode;
    OpenThreads::Mutex                              m_RunMutex;
    OpenThreads::Mutex                              m_RequestMutex;
};

#endif
