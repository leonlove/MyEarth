#ifndef I_STATE_IMPLEMENTER_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_STATE_IMPLEMENTER_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include "StateDefiner.h"

namespace cmm
{

class IStateQuerier;
class IStateImplementer : virtual public OpenSP::Ref
{
public:
    virtual void refreshObjectState(const IDList &objects, const std::string &strStateName, bool bState)    = 0;
};

}

#endif