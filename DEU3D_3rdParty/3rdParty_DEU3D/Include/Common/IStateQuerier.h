#ifndef I_STATE_QUERIER_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_STATE_QUERIER_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <map>
#include "StateDefiner.h"

namespace cmm
{
class IStateImplementer;

enum StateValue
{
    SV_Unknown,
    SV_Enable,
    SV_Disable
};

class IStateQuerier : virtual public OpenSP::Ref
{
public:
    virtual StateValue  queryObjectState(const ID &id, const std::string &strStateName) const                   = 0;
    virtual void        getObjectStates(const ID &id, std::map<std::string, bool> &objectstates) const          = 0;
    virtual void        setStateImplementer(IStateImplementer *pStateImplementer)                               = 0;
};

}

#endif
