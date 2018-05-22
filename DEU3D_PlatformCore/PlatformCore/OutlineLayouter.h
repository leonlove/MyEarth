#ifndef OUT_LINE_LAYOUTER_H_F359F2BD_B7C5_4372_9FE4_9D50F58A84E5_INCLUDE
#define OUT_LINE_LAYOUTER_H_F359F2BD_B7C5_4372_9FE4_9D50F58A84E5_INCLUDE

#include <osg/Group>
#include <osg/Stencil>
#include <osg/CullFace>
#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/Texture1D>
#include <osg/NodeCallback>

namespace
{
}

class OutlineLayouter : public osg::NodeCallback
{
public:
    explicit OutlineLayouter(void);
    OutlineLayouter(float fltWidth, const osg::Vec4& color);
    virtual ~OutlineLayouter(void);
    virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);
    void setWidth(float width);
    void setColor(const osg::Vec4 &vColor);

    /// Define render passes.
    void define_passes();
protected:
    void addPass(osg::StateSet* ss);

    bool define_techniques();
protected:
    float       m_fltWidth;
    osg::Vec4   m_clrColor;
    //osg::ref_ptr<OutlineTechnique> m_Technique;

    typedef std::vector<osg::ref_ptr<osg::StateSet> > Pass_list;
    Pass_list _passes;

    osg::ref_ptr<osg::LineWidth>    m_pLineWidth;
    osg::ref_ptr<osg::Material>     m_pMaterial;
};

#endif
