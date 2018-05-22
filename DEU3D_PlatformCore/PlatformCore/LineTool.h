#ifndef LINE_TOOL_H_9A2DD49F_EC38_4ADF_986F_F1ACBEA7FAC5_INCLUDE
#define LINE_TOOL_H_9A2DD49F_EC38_4ADF_986F_F1ACBEA7FAC5_INCLUDE

#include "ILineTool.h"
#include "PointTool.h"

class LineTool : virtual public ILineTool, public PointTool
{
public:
    explicit     LineTool(const std::string &strName);
    virtual     ~LineTool(void);

protected:  // methods from ILineTool
    virtual void    setLineWidth(double dblWidth);
    virtual double  getLineWidth(void) const                    {   return m_dblLineWidth;  }
    virtual void    setLineColor(const cmm::FloatColor &clr);
    virtual const   cmm::FloatColor &getLineColor(void) const   {   return m_clrLineColor;  }
    virtual const   std::string &getType(void) const            {   return LINE_TOOL;       }

protected:  // virtual methods from itself
    virtual bool operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:
    osg::Node *createLineNode(const osg::Vec3dArray *pLinePoints);

protected:
    double              m_dblLineWidth;
    cmm::FloatColor     m_clrLineColor;

};


#endif
