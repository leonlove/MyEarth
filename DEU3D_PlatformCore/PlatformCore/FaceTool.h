#ifndef FACE_TOOL_H_04A00044_AD6D_462B_ABDC_41F325DF4510_INCLUDE
#define FACE_TOOL_H_04A00044_AD6D_462B_ABDC_41F325DF4510_INCLUDE

#include "IFaceTool.h"
#include "PolylineTool.h"
#include <osg/Vec4>

class FaceTool : virtual public IFaceTool, public PolylineTool
{
public:
    explicit FaceTool(const std::string &strName);
    virtual ~FaceTool(void) = 0;

protected:  // methods from IFaceTool
    virtual void    setFaceColor(const cmm::FloatColor &color);
    virtual const   cmm::FloatColor &getFaceColor(void) const   {   return m_clrFaceColor;  }

protected:
    cmm::FloatColor     m_clrFaceColor;

};


#endif


