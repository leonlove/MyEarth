#ifndef POLYGON_TOOL_H_0E25ED26_0926_4C3A_BCBB_3B2C202D9811_INCLUDE
#define POLYGON_TOOL_H_0E25ED26_0926_4C3A_BCBB_3B2C202D9811_INCLUDE

#include "IPolygonTool.h"
#include "FaceTool.h"

class PolygonTool : public IPolygonTool, public FaceTool
{
public:
    explicit PolygonTool(const std::string &strName);
    virtual ~PolygonTool(void);

protected:  // methods from IRectTool
    virtual const   std::string &getType(void) const    {   return POLYGON_TOOL; }

protected:
    virtual bool operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:
    osg::Node *createPolygonGeode(const osg::Vec3Array *pVertexArray);

protected:
    
};

#endif

