#ifndef COLOR_STATE_H_ABEC21F0_60F4_4A82_AA50_BCFCDCF122ED_INCLUDE
#define COLOR_STATE_H_ABEC21F0_60F4_4A82_AA50_BCFCDCF122ED_INCLUDE

#include "IColorState.h"
#include "StateBase.h"

#include <osg/Material>
#include <common/StateDefiner.h>

class ColorState : public virtual IColorState, public StateBase
{
public:
    explicit ColorState(const std::string &strName);
    virtual ~ColorState(void);
public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_COLOR;  }
public:
    virtual const cmm::FloatColor getColor() const;
    virtual void setColor(const cmm::FloatColor &clr);
public:
    virtual bool applyState(osg::Node *pNode, bool bApply);
protected:
    class ColorCallBack : public osg::NodeCallback
    {
    public:
        explicit ColorCallBack(const osg::Vec4 &color);
        virtual ~ColorCallBack(void)  {}

    protected:
        virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

    public:
        void    setColor(const osg::Vec4 &color);

    protected:
        osg::ref_ptr<osg::StateSet> m_pStateSet;
        osg::ref_ptr<osg::Material> m_pMaterial;
    };

protected:
    osg::Vec4 m_color;
    osg::ref_ptr<ColorCallBack> m_pColorCallBack;
};

#endif
