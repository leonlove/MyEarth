#ifndef I_DIRECTION_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_DIRECTION_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <common/Common.h>
#include "IState.h"

class IDirectionState : virtual public IState
{
public:
    virtual cmm::FloatColor getColor(void) const        =   0;
    virtual void setColor(const cmm::FloatColor &clr)   =   0;

    virtual double getSpeed(void) const                 =   0;
    virtual void setSpeed(double dblSpeed)              =   0;

    virtual double getInterval(void) const              =   0;
    virtual void setInterval(double dblInterval)        =   0;
};

#endif