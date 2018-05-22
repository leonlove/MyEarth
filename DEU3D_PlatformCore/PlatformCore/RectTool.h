#ifndef RECT_TOOL_H_F6CA7E54_D7D7_4314_9DC3_0FFDFB583C70_INCLUDE
#define RECT_TOOL_H_F6CA7E54_D7D7_4314_9DC3_0FFDFB583C70_INCLUDE

#include "IRectTool.h"
#include "FaceTool.h"

class RectTool : public IRectTool, public FaceTool
{
public:
    explicit RectTool(const std::string &strName);
    virtual ~RectTool(void);

protected:  // methods from IToolBase
    virtual const   std::string &getType(void) const    {   return RECT_TOOL; }
    virtual void    setFaceColor(const cmm::FloatColor &color)
    {
        FaceTool::setFaceColor(color);
    }
    virtual const   cmm::FloatColor &getFaceColor(void) const
    {
        return FaceTool::getFaceColor();
    }

protected:
    virtual void    setRectNormal(float fltX, float fltY, float fltZ)    {    m_vNormal.set(fltX, fltY, fltZ);    }

protected:
    virtual bool        operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool        operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:
    osg::Node *createRectNode(const osg::Vec3dArray *pVertexArray);

protected:
    osg::Vec3   m_vNormal;
};

#endif
