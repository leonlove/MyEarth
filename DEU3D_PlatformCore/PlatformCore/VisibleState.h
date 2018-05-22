#ifndef VISIBLE_STATE_H_C0BC19D5_1D90_4B72_9FC1_00B937C2249A_INCLUDE
#define VISIBLE_STATE_H_C0BC19D5_1D90_4B72_9FC1_00B937C2249A_INCLUDE

#include "IVisibleState.h"
#include "StateBase.h"

class VisibleState : public virtual IVisibleState, public StateBase
{
public:
    explicit VisibleState(const std::string &strName);
    virtual ~VisibleState(void);

public:
    virtual bool  applyState(osg::Node *pNode, bool bApply);
    virtual const std::string &getType(void) const      {   return cmm::STATE_VISIBLE;  }
};

#endif
