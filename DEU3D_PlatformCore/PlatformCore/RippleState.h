#ifndef RIPPLE_STATE_H_INCLUDE
#define RIPPLE_STATE_H_INCLUDE

#include "IRippleState.h"
#include "StateBase.h"
#include <osg/Texture2D>
#include <time.h>

class RippleState : public virtual IRippleState, public StateBase
{
public:
    explicit RippleState(const std::string &strName);
    virtual ~RippleState(void);

public:
    virtual bool  applyState(osg::Node *pNode, bool bApply);
    virtual const std::string &getType(void) const      {   return cmm::STATE_RIPPLE;  }

    void apply(osg::Node *pNode, bool bApply);
protected:
    
    class UpdateCallbk : public osg::StateSet::Callback 
    {
    public:
        UpdateCallbk()
        {
            m_timeBegin = 0.0;
        }

        virtual void operator() (osg::StateSet* ss, osg::NodeVisitor* nv) 
        {
            float span = (float)(clock() - m_timeBegin) / CLOCKS_PER_SEC;
            ss->getOrCreateUniform("t", osg::Uniform::FLOAT)->set(span);
        }
        clock_t m_timeBegin;
    };

    osg::ref_ptr<UpdateCallbk>      m_pCallbk;
    osg::ref_ptr<osg::Program>      m_pProgram;
    osg::ref_ptr<osg::Image>        m_pOriginImg;
    osg::ref_ptr<osg::Image>        m_pRippleImg;
    osg::ref_ptr<osg::Texture2D>    m_pTexture;
};

#endif