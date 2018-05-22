#ifndef I_MEASURE_TOOL_H_C22329AF_7E54_4ABA_B9B6_75A2FFFE9CE7_INCLUDE
#define I_MEASURE_TOOL_H_C22329AF_7E54_4ABA_B9B6_75A2FFFE9CE7_INCLUDE

#include "IPolygonTool.h"

class IMeasureTool : virtual public IPolylineTool
{
public:
    enum MeasureType
    {
        Spatial,
        Horizontal,
        Vertical,
    };

    virtual void        setMeasureType(MeasureType eType)   = 0;
    virtual MeasureType getMeasureType(void) const          = 0;
};

#endif