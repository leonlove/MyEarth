#ifndef WIRE_FRAME_STATE_H_74E4EF7A_3A54_4197_A2B9_D7DBD6710907_INCLUDE
#define WIRE_FRAME_STATE_H_74E4EF7A_3A54_4197_A2B9_D7DBD6710907_INCLUDE

#include "IWireFrameState.h"
#include "StateBase.h"
#include <osg/NodeCallback>
#include <osg/StateSet>
#include <osg/LineWidth>
#include <osg/Material>

class WireFrameLayout : public osg::NodeCallback
{
public:
    explicit WireFrameLayout(float fltWidth, const osg::Vec4& color);
    virtual ~WireFrameLayout(void);

protected:
    virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

public:
    void setLineWidth(double dblWidth);
    double getLineWidth(void) const             {   return m_dblLineWidth;  }

    void setLineColor(const osg::Vec4 &color);
    const osg::Vec4 &getLineColor(void) const   {   return m_color; }

protected:
    osg::Vec4           m_color;
    double              m_dblLineWidth;
    osg::ref_ptr<osg::StateSet>     m_pWireFrameStateSet;
    osg::ref_ptr<osg::LineWidth>    m_pLineWidth;
    osg::ref_ptr<osg::Material>     m_pMaterial;
};


class WireFrameState : public virtual IWireFrameState, public StateBase
{
public:
    explicit WireFrameState(const std::string &strName);
    virtual ~WireFrameState(void);

public:
    virtual bool        applyState(osg::Node *pNode, bool bApply);
    virtual const       std::string &getType(void) const   {   return cmm::STATE_WIRE_FRAME;  }

protected:
    virtual void setLineColor(const cmm::FloatColor &color);
    virtual cmm::FloatColor getLineColor(void) const;

    virtual void setLineWidth(double dblWidth);
    virtual double getLineWidth(void) const;

protected:
    osg::ref_ptr<WireFrameLayout>   m_pWireFrameLayout;
};

#endif
