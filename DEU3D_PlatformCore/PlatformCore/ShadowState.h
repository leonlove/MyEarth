#ifndef SHADOW_STATE_H_INCLUDE
#define SHADOW_STATE_H_INCLUDE
#include "IShadowState.h"
#include "StateBase.h"
#include "Registry.h"

class ShadowState : public virtual IShadowState, public StateBase
{
public:
    explicit ShadowState(const std::string &strName);

    virtual ~ShadowState(void);

public:
    virtual bool  applyState(osg::Node *pNode, bool bApply);
    virtual const std::string &getType(void) const      {   return cmm::STATE_SHADOW;  }

};


#endif