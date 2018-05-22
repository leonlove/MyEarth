#ifndef I_WIRE_FRAME_STATE_H_7EF1AC0F_FC7C_45A0_8B70_EB6D471A2E0C_INCLUDE
#define I_WIRE_FRAME_STATE_H_7EF1AC0F_FC7C_45A0_8B70_EB6D471A2E0C_INCLUDE

#include "IState.h"
#include <common/Common.h>

class IWireFrameState : virtual public IState
{
public:
    virtual void setLineColor(const cmm::FloatColor &color) = 0;
    virtual cmm::FloatColor getLineColor(void) const = 0;

    virtual void setLineWidth(double dblWidth) = 0;
    virtual double getLineWidth(void) const = 0;
};

#endif

