#ifndef AREA_TOOL_H_INCLUDE
#define AREA_TOOL_H_INCLUDE

#include "IAreaTool.h"
#include "PolygonTool.h"

#include <osg/Geode>

class AreaTool : virtual public IAreaTool, public PolygonTool
{
public:
    explicit AreaTool(const std::string &strName);
    virtual ~AreaTool(void);

protected:  // virtual methods from IToolBase
    virtual const std::string           &getType(void) const    {   return AREA_TOOL;       }
    virtual void    setMap2Screen(bool bMap)                    {   m_bMap2Screen = false;   }

protected:  // virtual methods from itself
    virtual bool                        operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool                        operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
};

#endif
