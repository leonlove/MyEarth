
#include "EffectPagedLOD.h"

#include <osgUtil/CullVisitor>
#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osgUtil/CommonOSG.h>
#include <osg/ShapeDrawable>

EffectPagedLOD::EffectPagedLOD(osg::ref_ptr<osg::Switch> pEffectGroupSwitch)
{
    m_pEffectClipNode       = NULL;
    m_pEffectGroupSwitch    = pEffectGroupSwitch;

    m_effectType            = EF_TYPE_RAIN;
    m_effectLevel           = EF_LEVEL_MIDDLE;
    m_strEffectName         = "";
}

EffectPagedLOD::~EffectPagedLOD()
{
    removeEffectClipNode();
}

void EffectPagedLOD::setEffectType(EffectType effectType)
{
    m_effectType = effectType;
}

void EffectPagedLOD::setEffectLevel(EffectLevel effectLevel)
{
    if (m_effectLevel == effectLevel)
    {
        return;
    }
    m_effectLevel = effectLevel;

    if (NULL == m_pEffectClipNode)
    {
        return;
    }

    osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode= dynamic_cast<osgParticle::PrecipitationEffect*>(m_pEffectClipNode->getChild(0));
    if (NULL == pPrecipitationNode)
    {
        return;
    }

    setEffectParam(pPrecipitationNode);
}

void EffectPagedLOD::setEffectName(const std::string& strEffectName)
{
    m_strEffectName = strEffectName;
}

EffectType  EffectPagedLOD::getEffectType()
{
    return m_effectType;
}

EffectLevel EffectPagedLOD::getEffectLevel()
{
    return m_effectLevel;
}

std::string EffectPagedLOD::getEffectName()
{
    return m_strEffectName;
}

void EffectPagedLOD::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if(!pCullVisitor)
    {
        osg::Group::traverse(nv);
        return;
    }

    switch(nv.getTraversalMode())
    {
    case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
        std::for_each(_children.begin(), _children.end(), osg::NodeAcceptOp(nv));
        break;
    case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float required_range = 0.0f;
            if (_rangeMode == DISTANCE_FROM_EYE_POINT)
            {
                required_range = nv.getDistanceToViewPoint(getCenter());
            }
            else
            {
                osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
                if (cullStack)
                    required_range = cullStack->clampedPixelSize(getBound());
                else
                {
                    for(unsigned int i = 0u ; i < _rangeList.size(); ++i)
                    {
                        const MinMaxPair &range = _rangeList[i];
                        required_range = osg::maximum(required_range, range.first);
                    }
                }
            }

            if (getRadius() < required_range)
            {
                break;
            }
            
            if (NULL != m_pEffectClipNode)
            {
                if (!m_pEffectGroupSwitch->getChildValue(m_pEffectClipNode))
                {
                    m_pEffectGroupSwitch->setAllChildrenOff();
                    m_pEffectGroupSwitch->setChildValue(m_pEffectClipNode, true);
                }
                break;
            }
            
            m_pEffectClipNode = createEffectClipNode();
            m_pEffectGroupSwitch->addChild(m_pEffectClipNode.get(), false);

            break;
        }
    default:
        break;
    }

    return;
}

osg::ref_ptr<osg::ClipNode> EffectPagedLOD::createEffectClipNode()
{
    osg::Vec3d ptCenter = getCenter();

    osg::BoundingSphere bs;
    bs.set(ptCenter, getRadius());
    float fWidth = bs.radius()*2.0f;

    double dMinX = (2*ptCenter.x() - fWidth) / 2.0;
    double dMaxX = dMinX + fWidth;
    double dMinY = (2*ptCenter.y() - fWidth) / 2.0;
    double dMaxY = dMinY + fWidth;
    double dMinZ = (2*ptCenter.z() - fWidth) / 2.0;
    double dMaxZ = dMinZ + fWidth;

    osg::ref_ptr<osg::ClipNode> pEffectClipNode = new osg::ClipNode;
    pEffectClipNode->addClipPlane(new osg::ClipPlane(0, 1.0,0.0,0.0,-dMinX));
    pEffectClipNode->addClipPlane(new osg::ClipPlane(1, -1.0,0.0,0.0,dMaxX));
    pEffectClipNode->addClipPlane(new osg::ClipPlane(2, 0.0,1.0,0.0,-dMinY));
    pEffectClipNode->addClipPlane(new osg::ClipPlane(3, 0.0,-1.0,0.0,dMaxY));
    pEffectClipNode->addClipPlane(new osg::ClipPlane(4, 0.0,0.0,1.0,-dMinZ));
    pEffectClipNode->addClipPlane(new osg::ClipPlane(5, 0.0,0.0,-1.0,dMaxZ));

    pEffectClipNode->addChild(createPrecipitationNode());

    return pEffectClipNode;
}

osg::ref_ptr<osgParticle::PrecipitationEffect> EffectPagedLOD::createPrecipitationNode()
{
    osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode = new osgParticle::PrecipitationEffect();

    switch (m_effectType)
    {
    case EF_TYPE_RAIN:
        {
            pPrecipitationNode->rain(0.5f);
            break;
        }
    case EF_TYPE_SNOW:
        {
            pPrecipitationNode->snow(0.5f);
            break;
        }
    default:
        break;
    }

    setEffectParam(pPrecipitationNode);
    return pPrecipitationNode;
}

void EffectPagedLOD::setEffectParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode)
{
    if (NULL == pPrecipitationNode)
    {
        return;
    }

    if (m_effectType == EF_TYPE_RAIN)
    {
        switch (m_effectLevel)
        {
        case EF_LEVEL_LOW:
            setParam(pPrecipitationNode, EF_TYPE_RAIN, 30.0, 30.0, -8.0, 1.6/100.0);
            break;
        case EF_LEVEL_MIDDLE:
            setParam(pPrecipitationNode, EF_TYPE_RAIN, 30.0, 30.0, -10.0, 2.2/100.0);
            break;
        case EF_LEVEL_HIGH:
            setParam(pPrecipitationNode, EF_TYPE_RAIN, 30.0, 30.0, -14.0, 3.1/100.0);
            break;
        default:
            break;
        }
    }
    else if (m_effectType == EF_TYPE_SNOW)
    {
        switch (m_effectLevel)
        {
        case EF_LEVEL_LOW:
            setParam(pPrecipitationNode, EF_TYPE_SNOW, 15.0, 15.0, -1.0, 2/100.0);
            break;
        case EF_LEVEL_MIDDLE:
            setParam(pPrecipitationNode, EF_TYPE_SNOW, 15.0, 15.0, -2.0, 4/100.0);
            break;
        case EF_LEVEL_HIGH:
            setParam(pPrecipitationNode, EF_TYPE_SNOW, 15.0, 15.0, -3.0, 7/100.0);
            break;
        default:
            break;
        }
    }

    return;
}

bool EffectPagedLOD::setParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitationNode, EffectType nEffectType, double dNearTransition, double dFarTransition, double dParticleSpeed, double dParticleSize)
{
    if (NULL == pPrecipitationNode)
    {
        return false;
    }

    pPrecipitationNode->setNearTransition(dNearTransition);
    pPrecipitationNode->setFarTransition(dFarTransition);
    if (EF_TYPE_RAIN == nEffectType)
    {
        pPrecipitationNode->setParticleSpeed(dParticleSpeed);
        pPrecipitationNode->setParticleColor(osg::Vec4(0.7,0.7,0.7,1.0));
    }
    else
    {
        pPrecipitationNode->setParticleColor(osg::Vec4(1.0,1.0,1.0,1.0));
    }
    pPrecipitationNode->setParticleSize(dParticleSize);

    return true;
}

void EffectPagedLOD::removeEffectClipNode()
{
    if (NULL==m_pEffectClipNode || NULL==m_pEffectGroupSwitch)
    {
        return;
    }

    unsigned int nPos = m_pEffectGroupSwitch->getChildIndex(m_pEffectClipNode);
    m_pEffectGroupSwitch->removeChildren(nPos, 1);

    m_pEffectClipNode->removeChildren(0, m_pEffectClipNode->getNumChildren());

    for (unsigned int i=0; i<6; i++)
    {
        m_pEffectClipNode->removeClipPlane((unsigned int)0);
    }

    m_pEffectClipNode = NULL;
}

