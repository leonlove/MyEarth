#ifndef POINT_TOOL_H_02009637_0E0B_4430_A948_62FE83613A89_INCLUDE
#define POINT_TOOL_H_02009637_0E0B_4430_A948_62FE83613A89_INCLUDE

#include "IPointTool.h"
#include "ToolBase.h"
#include "common/Common.h"

namespace osg
{
    class Geode;
};

class PointTool : virtual public IPointTool, public ToolBase
{
public:
    explicit     PointTool(const std::string &strName);
    virtual     ~PointTool(void);

public: // methods from IPointTool
    virtual void    setPointSize(double dblPointlSize);
    virtual double  getPointSize(void) const                    {   return m_dblPointSize;  }
    virtual void    setPointColor(const cmm::FloatColor &color);
    virtual const   cmm::FloatColor &getPointColor(void) const  {   return m_clrPointColor; }

    virtual void    enableDrawPoint(bool bDraw = true)      {   m_bDrawPoint = bDraw;   }
    virtual bool    isEnableDrawPoint(void) const           {   return m_bDrawPoint;    }
    virtual const   std::string &getType(void) const        {   return POINT_TOOL;   }
    virtual void    clearArtifact(void);

protected:
    virtual bool    operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual void    onDeactive(void);

protected:
    osg::Node  *createPointNode(const osg::Vec3dArray *pVertexArray);

protected:
    double              m_dblPointSize;
    cmm::FloatColor     m_clrPointColor;
    bool                m_bDrawPoint;
    osg::ref_ptr<osg::Vec3dArray>   m_pVertexArray;
};

#endif
