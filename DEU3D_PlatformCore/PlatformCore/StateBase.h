#ifndef STATE_BASE_H_90648D78_1729_49EF_B3BB_1C2CA4D6C5D4_INCLUDE
#define STATE_BASE_H_90648D78_1729_49EF_B3BB_1C2CA4D6C5D4_INCLUDE

#pragma warning (disable : 4250)

#include <OpenSP/Ref.h>
#include <osg/Node>

#include "IState.h"
#include <Common/IStateQuerier.h>
#include <common/StateDefiner.h>

class StateBase : virtual public IState
{
public:
    explicit StateBase(const std::string &strName);
    virtual ~StateBase(void) = 0;

public:
    virtual unsigned    getPriority(void) const                 {   return m_nPriority; }
    virtual void        setPriority(unsigned nPriority)         {   m_nPriority = nPriority;    }
    virtual const       std::string &getName(void) const   {   return m_strName;   }
    virtual bool        applyState(osg::Node *pNode, bool bApply) = 0;

protected:
    std::string     m_strName;
    unsigned        m_nPriority;
};

#endif
