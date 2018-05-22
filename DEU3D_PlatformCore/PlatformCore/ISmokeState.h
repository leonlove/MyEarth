#ifndef I_SMOKE_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_SMOKE_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <common/Common.h>
#include "IState.h"

class ISmokeState : virtual public IState
{
public:
    virtual void setSmokeImageFile(const std::string &strSmokeFile)         = 0;
    virtual void setSmokeSize(double dblSize)                               = 0;
    virtual const cmm::FloatColor getSmokeColor() const                     = 0;
    virtual void setSmokeColor(const cmm::FloatColor &clr)                  = 0;
    virtual void setSmokeDirection(double dblAzimuth, double dblPitch)      = 0;
    virtual void getSmokeDirection(double &dblAzimuth, double &dblPitch)    = 0;
};

#endif