#ifndef I_LINE_TOOL_H_F0F96FB8_D24B_416A_839A_E6F0519CFE85_INCLUDE
#define I_LINE_TOOL_H_F0F96FB8_D24B_416A_839A_E6F0519CFE85_INCLUDE

#include "IPointTool.h"

class ILineTool : virtual public IPointTool
{
public:
    virtual void    setLineWidth(double dblWidth)               = 0;
    virtual double  getLineWidth(void) const                    = 0;
    virtual void    setLineColor(const cmm::FloatColor &color)  = 0;
    virtual const   cmm::FloatColor &getLineColor(void) const   = 0;
};

#endif

