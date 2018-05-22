#ifndef I_COLOR_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_COLOR_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <common/Common.h>

#include "IState.h"

class IColorState : virtual public IState
{
public:
    virtual const cmm::FloatColor getColor() const     =   0;
    virtual void setColor(const cmm::FloatColor &clr)      =   0;
};

#endif