#ifndef I_POINT_TOOL_H_C22329AF_7E54_4ABA_B9B6_75A2FFFE9CE7_INCLUDE
#define I_POINT_TOOL_H_C22329AF_7E54_4ABA_B9B6_75A2FFFE9CE7_INCLUDE

#include "IToolBase.h"
#include "Common/Common.h"

class IPointTool : virtual public IToolBase
{
public:
    virtual void    setPointSize(double dblPointlSize)              = 0;
    virtual double  getPointSize(void) const                        = 0;
    virtual void    setPointColor(const cmm::FloatColor &color)     = 0;
    virtual const   cmm::FloatColor &getPointColor(void) const      = 0;
    virtual void    enableDrawPoint(bool bDraw = true)              = 0;
    virtual bool    isEnableDrawPoint(void) const                   = 0;
};


#endif