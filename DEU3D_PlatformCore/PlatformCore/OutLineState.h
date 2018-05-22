#ifndef OUT_LINE_STATE_H_D6BC5A54_363E_404A_91FB_0CD38E206A6B_INCLUDE
#define OUT_LINE_STATE_H_D6BC5A54_363E_404A_91FB_0CD38E206A6B_INCLUDE

#include "IOutLineState.h"
#include "StateBase.h"

#include "OutlineLayouter.h"
#include "IStateManager.h"

class OutLineState : public virtual IOutLineState, public StateBase
{
public:
    explicit OutLineState(const std::string &strName);
    virtual ~OutLineState(void);

public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_OUTLINE;  }

public:
    virtual double getLineWidth();
    virtual void setLineWidth(double dblLineWidth);
    virtual const cmm::FloatColor getColor() const;
    virtual void setColor(const cmm::FloatColor &clr);

public:
    virtual bool applyState(osg::Node *pNode, bool bApply);

protected:
    osg::ref_ptr<OutlineLayouter> m_pOutLineLayouter;
    osg::Vec4                   m_color;
};


#endif
