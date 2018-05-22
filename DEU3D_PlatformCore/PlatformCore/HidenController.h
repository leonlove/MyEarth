#ifndef HIDEN_CONTROLLER_H_C6AEA691_7390_4EF8_8560_67C5109AD884_INCLUDE
#define HIDEN_CONTROLLER_H_C6AEA691_7390_4EF8_8560_67C5109AD884_INCLUDE

#include <osg/Node>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>

class FileReadInterceptor;
class HidenController : public OpenSP::Ref
{
public:
    explicit HidenController(void);
    virtual ~HidenController(void);

public:
    inline void setRootNode(osg::Node *pRootNode) {    m_pRootNode = pRootNode;    }
    void hiden(const IDList &vecHidenList, bool bHiden = true);

public:
    void destroyHidenThread();

protected:

    class HidenThread : public OpenThreads::Thread
    {
    public:
        HidenThread(osg::Node *pNode, std::set<ID> &idList, bool bHiden) : 
            m_pRootNode(pNode),
            m_idList(idList),
            m_bHiden(bHiden)
        {
        };

        ~HidenThread(void) {};
        virtual void run(void);
    public:
        void stop() {    ++m_bStop;    }
    protected:
        void hiden(osg::Node *pNode);
    protected:
        OpenThreads::Atomic m_bStop;
        bool                m_bHiden;
        osg::Node           *m_pRootNode;
        std::set<ID>        m_idList;
    };

protected:
    HidenThread         *m_pHidenThread;
    std::set<ID>        m_displayList;
    osg::Node           *m_pRootNode;
};

#endif
