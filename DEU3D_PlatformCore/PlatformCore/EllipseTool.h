#ifndef ELLIPSE_TOOL_H_85185001_B137_4325_8C4B_8277C7D6B885_INCLUDE
#define ELLIPSE_TOOL_H_85185001_B137_4325_8C4B_8277C7D6B885_INCLUDE

#include "IEllipseTool.h"
#include "FaceTool.h"

class EllipseTool : public IEllipseTool, public FaceTool
{
public:
    explicit EllipseTool(const std::string &strName);
    virtual ~EllipseTool(void);

protected:  // methods from IEllipseTool
    virtual const   std::string &getType(void) const    {   return ELLIPSE_TOOL; }
    virtual void    setFaceColor(const cmm::FloatColor &color)
    {
        FaceTool::setFaceColor(color);
    }
    virtual const   cmm::FloatColor &getFaceColor(void) const
    {
        return FaceTool::getFaceColor();
    }

protected:
    virtual bool handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
};

#endif

