#ifndef I_FACE_TOOL_H_68FCC37B_2706_4D90_95B0_A76AAB7AB483_INCLUDE
#define I_FACE_TOOL_H_68FCC37B_2706_4D90_95B0_A76AAB7AB483_INCLUDE

#include "IPolylineTool.h"

class IFaceTool : virtual public IPolylineTool
{
public:
    virtual void    setFaceColor(const cmm::FloatColor &color) = 0;
    virtual const   cmm::FloatColor &getFaceColor(void) const = 0;
};


#endif
