#ifndef LOD_FIXER_H_BB6A57DB_81C0_47CD_AD78_0FE68C4421ED_INCLUDE
#define LOD_FIXER_H_BB6A57DB_81C0_47CD_AD78_0FE68C4421ED_INCLUDE

#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osgGA/GUIEventHandler>

class LODFixer : public osgGA::GUIEventHandler
{
public:
    explicit LODFixer(double dblVTileRangeRatio);
    virtual ~LODFixer(void);

public:
    void setMinRangeRatio(float fltMin)    {   m_fltMinRangeRatio = fltMin;    }

protected:
    virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

protected:
    float           m_fltCameraCosFovy;
    float           m_fltCameraCosFovy_2;
    float           m_fltMinRangeRatio;
};

#endif
