#ifndef EFFECT_PAGEDLOD_INCLUDE
#define EFFECT_PAGEDLOD_INCLUDE

#include <osg/PagedLOD>
#include <osg/ClipNode>
#include <osg/Switch>

#include <osgParticle/PrecipitationEffect>

#include "IPlatformCore.h"

class EffectPagedLOD : public osg::PagedLOD
{
public:
    EffectPagedLOD(osg::ref_ptr<osg::Switch> pEffectGroupSwitch);
    ~EffectPagedLOD();

    virtual void traverse(osg::NodeVisitor &nv);

public:
    void                                            setEffectType(EffectType effectType);
    void                                            setEffectLevel(EffectLevel effectLevel);
    void                                            setEffectName(const std::string& strEffectName);

    EffectType                                      getEffectType();
    EffectLevel                                     getEffectLevel();
    std::string                                     getEffectName();

protected:
    osg::ref_ptr<osg::ClipNode>                     createEffectClipNode();
    void                                            removeEffectClipNode();
    osg::ref_ptr<osgParticle::PrecipitationEffect>  createPrecipitationNode();
    
    void                                            setEffectParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode);
    bool                                            setParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode, EffectType nEffectType, double dNearTransition, double dFarTransition, double dParticleSpeed, double dParticleSize);

protected:
    EffectType                      m_effectType;
    EffectLevel                     m_effectLevel;
    std::string                     m_strEffectName;

    osg::ref_ptr<osg::ClipNode>     m_pEffectClipNode;
    osg::ref_ptr<osg::Switch>       m_pEffectGroupSwitch;

};

#endif