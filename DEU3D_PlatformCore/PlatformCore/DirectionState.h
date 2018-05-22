#ifndef DIRECTION_STATE_H_ABEC21F0_60F4_4A82_AA50_BCFCDCF122ED_INCLUDE
#define DIRECTION_STATE_H_ABEC21F0_60F4_4A82_AA50_BCFCDCF122ED_INCLUDE

#include "IDirectionState.h"
#include "StateBase.h"

#include <osg/Texture1D>
#include <osg/TexMat>
#include <common/StateDefiner.h>

class DirectionState : public virtual IDirectionState, public StateBase
{
public:
    explicit DirectionState(const std::string &strName);
    virtual ~DirectionState(void);
public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_DIRECTION;  }
public:
    virtual cmm::FloatColor getColor(void) const;
    virtual void setColor(const cmm::FloatColor &clr);

    virtual double getSpeed(void) const;
    virtual void setSpeed(double dblSpeed);

    virtual double getInterval(void) const;
    virtual void setInterval(double dblInterval);

protected:
    virtual bool applyState(osg::Node *pNode, bool bApply);

    class DirectionCallBack : public osg::NodeCallback
    {
    public:
        explicit DirectionCallBack(void);
        virtual ~DirectionCallBack()  {}

    public:
        cmm::FloatColor getColor(void) const;
        void setColor(const cmm::FloatColor &clr);

        double getSpeed(void) const;
        void setSpeed(double dblSpeed);

        double getInterval(void) const;
        void setInterval(double dblInterval);

        void applyState(osg::StateSet *pStateSet, bool bApply);

    protected:
        virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

    protected:
        osg::ref_ptr<osg::Program>      m_pProgram;
        osg::ref_ptr<osg::Uniform>      m_pColorUniform;
        osg::ref_ptr<osg::Uniform>      m_pSpeedUniform;
        osg::ref_ptr<osg::Uniform>      m_pStepUniform;
        osg::ref_ptr<osg::Uniform>      m_pTimeUniform;
        unsigned int    m_nFrameNum;
        OpenThreads::Mutex              m_mutex;
    };

    friend class DirectionCallBack;
protected:
    osg::ref_ptr<DirectionCallBack> m_pDirectionCallBack;
};

#endif

