#ifndef VIEW_STATE_H_INCLUDE
#define VIEW_STATE_H_INCLUDE

#include <osg/NodeCallback>

#include "IViewState.h"
#include "StateBase.h"

class ViewState : public virtual IViewState, public StateBase
{
public:
    explicit ViewState(const std::string &strName);
    virtual ~ViewState(void);
public:
    virtual void setViews(const unsigned int nViews)
    {
        m_pCallBack = new OutofViewCallBack(nViews);
    }

    virtual unsigned int getViews()   {   return m_pCallBack->getViews(); }
public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_VIEWS;  }
    virtual bool        applyState(osg::Node *pNode, bool bApply);

protected:
    class OutofViewCallBack : public osg::NodeCallback
    {
    public:
        explicit OutofViewCallBack(const unsigned int nViews);
        virtual ~OutofViewCallBack(void);
        virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

        unsigned int getViews() {   return m_nViews;    }
    protected:
        unsigned int m_nViews;
    };

protected:
    //unsigned int                    m_nViews;
    osg::ref_ptr<OutofViewCallBack> m_pCallBack;
};

#endif

