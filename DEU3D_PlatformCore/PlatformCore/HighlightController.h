#ifndef HIGHLIGHT_CONTROLLER_H
#define HIGHLIGHT_CONTROLLER_H 1

#include <osg/Node>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>

class HighlightController : public OpenSP::Ref
{
public:
    explicit HighlightController(void);
    virtual ~HighlightController(void);

public:
    void highlight(const IDList &vecHidenList);
    inline void setRootNode(osg::Node *pNode)
    {
        m_pRootNode = pNode;
    }
public:
    void destroyHidenThread();

protected:
    class HighlightThread : public OpenThreads::Thread
    {
    public:
        HighlightThread(osg::Node *pNode, std::set<ID> &idList) : 
          m_pRootNode(pNode),
              m_idList(idList)
          {
          };

          ~HighlightThread(void) {};
          virtual void run(void);
    public:
        void stop() {    ++m_bStop;    }
    protected:
        void highlight(osg::Node *pNode);
    protected:
        OpenThreads::Atomic m_bStop;
        bool                m_bHighlight;
        osg::Node           *m_pRootNode;
        std::set<ID>        m_idList;
    };

protected:
    HighlightThread *m_pHidenThread;
    osg::Node       *m_pRootNode;
};

#endif
