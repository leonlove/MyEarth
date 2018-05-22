#ifndef EDITING_TOOL_H_02009637_0E0B_4430_A948_62FE83613A89_INCLUDE
#define EDITING_TOOL_H_02009637_0E0B_4430_A948_62FE83613A89_INCLUDE

#include "IEditingTool.h"
#include "ToolBase.h"

class EditingTool : virtual public IEditingTool, public ToolBase
{
public:
    explicit EditingTool(const std::string &strName);
    virtual ~EditingTool(void);

protected:
    virtual const std::string           &getType(void) const    {   return EDITING_TOOL; }

    virtual bool setEditingTarget(const ID &strID);
    virtual const ID &getEditingTarget(void) const;
    virtual void setUnit(double dblUnit);
    virtual double getUnit(void);


    virtual bool    operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) {    return false;   }
    virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
protected:
    //std::string m_strType;
    //osg::Vec3d  m_vTrans;
    //osg::Vec3d  m_vScale;
    double      m_dblUnit;
    ID          m_TargetID;
    osg::ref_ptr<osg::Node> m_pEditingNode;
    int m_nEditingIndex;
    osg::Vec4   m_clrPoint;
};

#endif