#ifndef I_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <OpenSP/Ref.h>
#include <string>

class IState : virtual public OpenSP::Ref
{
public:
    virtual unsigned    getPriority(void) const               = 0;
    virtual void        setPriority(unsigned nPriority)       = 0;
    virtual const       std::string &getType(void) const = 0;
    virtual const       std::string &getName(void) const = 0;
};

#endif