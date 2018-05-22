#pragma once

#include <IDProvider/ID.h>
#include <osg/Object>
#include <osg/observer_ptr>
#include <OpenThreads/Thread>
#include <OpenSP/Ref.h>
#include <map>

class SharedObjectPool : public OpenSP::Ref
{
public:
    SharedObjectPool(void);
    ~SharedObjectPool(void);

public:
    void addObject(const ID &id, osg::Object *pObject);
    bool findObject(const ID &id, osg::ref_ptr<osg::Object> &pObject);

    void trimePool();

protected:
    class SharedObjectThread : public osg::Referenced, public OpenThreads::Thread
    {
    public:
        SharedObjectThread(SharedObjectPool *pPool);

        void setDone(bool done) { m_done.exchange(done ? 1 : 0); }
        bool getDone() const { return m_done != 0; }

        virtual int cancel();

        virtual void run();

    protected:

        virtual ~SharedObjectThread();

        OpenThreads::Atomic m_done;
        SharedObjectPool    *m_pPool;

    };

protected:
    OpenThreads::Mutex m_mtxObjectPool;
    std::map<const ID, osg::observer_ptr<osg::Object> > m_mapObject;
    osg::ref_ptr<SharedObjectThread> m_pThread;
};

