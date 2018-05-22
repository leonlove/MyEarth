#ifndef TOPMOST_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define TOPMOST_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include "ITopmostState.h"
#include "StateBase.h"

class TopmostState: public virtual ITopmostState, public StateBase
{
public:
    explicit TopmostState(const std::string &strName);
    virtual ~TopmostState(void);
public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_TOPMOST;  }
public:
    virtual bool applyState(osg::Node *pNode, bool bApply);
};

#endif