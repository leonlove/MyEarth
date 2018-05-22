#ifndef MEASURE_TOOL_H_INCLUDE
#define MEASURE_TOOL_H_INCLUDE

#include "IMeasureTool.h"
#include "PolylineTool.h"

#include <osg/Geode>

class MeasureTool : virtual public IMeasureTool, public PolylineTool
{
public:
    explicit MeasureTool(const std::string &strName);
    virtual ~MeasureTool(void);

protected:  // virtual methods from IToolBase
    virtual const std::string           &getType(void) const    {   return MEASURE_TOOL; }
    virtual void    setMap2Screen(bool bMap)                    {   m_bMap2Screen = false;   }

protected:
    virtual void                        setMeasureType(IMeasureTool::MeasureType eType);
    virtual IMeasureTool::MeasureType   getMeasureType(void) const;

protected:  // virtual methods from itself
    virtual bool    operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual void    clearArtifact(void);


    virtual void onActive(void)
    {
        if(m_pToolNode->getNumChildren() == 0)
        {
            m_pToolNode->addChild(m_pCurrentArtifactNode);
        }
    }


    virtual void onDeactive(void)
    {
    }
protected:
    osg::Group                          *createMeasureGeode(const osg::Vec3dArray *pLinePoints, bool bNeedSend, bool bFinished);

protected:
    IMeasureTool::MeasureType           m_eMeasureType;
    osg::ref_ptr<osg::Group>            m_pCurrentNode;
};

#endif
